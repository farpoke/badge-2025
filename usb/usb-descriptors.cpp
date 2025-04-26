#include <tusb.h>

// C.f. https://github.com/hathach/tinyusb/blob/master/examples/device/cdc_msc/src/usb_descriptors.c

constexpr tusb_desc_device_t device_descriptor = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = DEVICE_VERSION_BCD,

    .iManufacturer      = USB_STRING_VENDOR,
    .iProduct           = USB_STRING_PRODUCT,
    .iSerialNumber      = USB_STRING_SERIAL,

    .bNumConfigurations = 1
};

#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_MSC_DESC_LEN)

constexpr uint8_t fs_config_descriptor[] = {

    // Configuration Descriptor
    TUD_CONFIG_DESCRIPTOR(
        1,                      // Configuration number
        USB_IF_NUM_TOTAL,       // Interface count
        0,                      // Configuration name string index
        CONFIG_TOTAL_LEN,       // Total descriptor length
        0x00,                   // Attibutes
        50                      // Max power (mA)
    ),

    // CDC Interface Descriptor
    TUD_CDC_DESCRIPTOR(
        USB_IF_NUM_CDC,         // Interface number
        USB_STRING_CDC_IF,      // Interface name string index
        USB_EP_NUM_CDC_NOTIF,   // Notification endpoint
        8,                      // Notification max packet size
        USB_EP_NUM_CDC_OUT,     // OUT endpoint
        USB_EP_NUM_CDC_IN,      // IN endpoint
        64                      // OUT/IN endpoint max packet size
    ),

    // MSC Interface Descriptor
    TUD_MSC_DESCRIPTOR(
        USB_IF_NUM_MSC,         // Interface number
        USB_STRING_MSC_IF,      // Interface name string index
        USB_EP_NUM_MSC_OUT,     // OUT endpoint
        USB_EP_NUM_MSC_IN,      // IN endpoint
        64                      // OUT/IN endpoint max packet size
    ),
};

uint8_t const * tud_descriptor_device_cb() {
    return reinterpret_cast<const uint8_t*>(&device_descriptor);
}

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return fs_config_descriptor;
}

const char16_t* get_string(int index) {
    switch (index) {
        case USB_STRING_VENDOR:     return u"HackGBGay";
        case USB_STRING_PRODUCT:    return u"Badge-2025";
        case USB_STRING_SERIAL:     return u"[Serial Number]";
        case USB_STRING_CDC_IF:     return u"Badge COM Port";
        case USB_STRING_MSC_IF:     return u"Badge Mass Storage";
        default:
            return nullptr;
    }
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    const auto string = get_string(index);
    (void)langid;
    return reinterpret_cast<const uint16_t*>(string);
}
