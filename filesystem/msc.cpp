#include "../usb/usb.hpp"

#include <tusb.h>

extern const int DISK_IMAGE_SIZE;
extern const int DISK_IMAGE_BLOCK_COUNT;
extern const uint8_t DISK_IMAGE[];

static bool ejected = false;

void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {
    (void)lun;
    memcpy(vendor_id, "HackGBG ", 8);
    memcpy(product_id, "Badge-2025      ", 16);
    memcpy(product_rev, "1.0 ", 4);
}

bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject) {
    (void) lun;
    (void) power_condition;

    if (load_eject) {
        if (start) {
            // load disk storage
        } else {
            // unload disk storage
            ejected = true;
        }
    }

    return true;
}

bool tud_msc_test_unit_ready_cb(uint8_t lun) {
    (void)lun;

    if (ejected) {
        // If we've been told to eject, then tell the host that we're not ready any more.
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
        return false;
    }

    return true;
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size) {
    (void)lun;
    *block_count = DISK_IMAGE_BLOCK_COUNT;
    *block_size  = USB_MSC_BLOCK_SIZE;
}

bool tud_msc_is_writable_cb(uint8_t lun) {
    (void)lun;
    return false;
}

int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
    (void)lun;

    if (lba >= DISK_IMAGE_BLOCK_COUNT) {
        printf("! MSC: Attempt to read out of bounds (%d, %d, %d, %d)\n", lun, lba, offset, bufsize);
        return -1;
    }

    const auto idx = lba * USB_MSC_BLOCK_SIZE + offset;
    if ((idx + bufsize) > DISK_IMAGE_SIZE) {
        printf("! MSC: Attempt to read out of bounds (%d, %d, %d, %d)\n", lun, lba, offset, bufsize);
        return -1;
    }

    memcpy(buffer, DISK_IMAGE + idx, bufsize);
    return static_cast<int32_t>(bufsize);
}

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
    printf("! MSC: Attempt to write (%d, %d, %d, %d)\n", lun, lba, offset, bufsize);
    (void)buffer;
    return -1;
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize) {
    (void)lun;
    (void)scsi_cmd;
    (void)buffer;
    (void)bufsize;
    // Set SCSI sense to indicate illegal/unsupported command.
    tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
    return -1;
}
