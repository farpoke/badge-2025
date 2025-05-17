#include "storage.hpp"

#include <cstdio>
#include <cstring>
#include <memory>

#include <hardware/flash.h>
#include <hardware/sync.h>
#include <pico/flash.h>
#include <pico/stdio.h>

#include <badge/badge-2025.h>
#include <badge/lcd.hpp>
#include <utils/crc.hpp>

#include "pico/time.h"


namespace storage
{

    namespace
    {
        constexpr int STORAGE_SECTORS = 2;
        constexpr int STORAGE_SIZE = FLASH_SECTOR_SIZE * STORAGE_SECTORS;
        constexpr intptr_t STORAGE_BASE_OFFSET = BADGE_FLASH_SIZE - STORAGE_SIZE;
        constexpr int STORAGE_UNITS = STORAGE_SIZE / STORAGE_UNIT_SIZE;

        struct StorageUnit {
            uint32_t crc = {};
            StorageData data = {};
            uint8_t _padding[STORAGE_UNIT_SIZE - sizeof(StorageData) - 4] = {};

            [[nodiscard]] uint32_t compute_crc() const {
                return utils::crc32({reinterpret_cast<const uint8_t*>(this) + 4, STORAGE_UNIT_SIZE - 4});
            }
        };

        static_assert(sizeof(StorageUnit) == STORAGE_UNIT_SIZE);

        constexpr intptr_t get_offset(int unit_index) {
            assert(unit_index >= 0 && unit_index < STORAGE_UNITS);
            return STORAGE_BASE_OFFSET + static_cast<intptr_t>(unit_index * STORAGE_UNIT_SIZE);
        }

        template<typename T>
        const T *unit_ptr(int unit_index) {
            return reinterpret_cast<const T *>(XIP_BASE + get_offset(unit_index));
        }

        bool is_valid(int unit_index) {
            const auto unit = unit_ptr<StorageUnit>(unit_index);
            return unit->crc == unit->compute_crc();
        }

        bool is_erased(int unit_index) {
            auto *ptr = unit_ptr<uint8_t>(unit_index);
            for (size_t i = 0; i < STORAGE_UNIT_SIZE; i++)
                if (ptr[i] != 0xFF)
                    return false;
            return true;
        }

        int _currentUnit;

        StorageUnit _ramUnit = {};

    } // namespace

    StorageData *ram_data = nullptr;

    void init() {
        for (_currentUnit = 0; _currentUnit < STORAGE_UNITS; _currentUnit++) {
            if (is_valid(_currentUnit)) {
                printf("> Loading stored data from unit %d\n", _currentUnit);
                _ramUnit = *unit_ptr<StorageUnit>(_currentUnit);
                break;
            }
        }
        if (_currentUnit == STORAGE_UNITS) {
            printf("> No valid stored data\n");
            erase();
        }
        ram_data = &_ramUnit.data;
    }

    void erase() {
        printf("! Erasing all storage\n");
        // Erase all FLASH space allocated to storage.
        flash_safe_execute([](auto){
            flash_range_erase(STORAGE_BASE_OFFSET, STORAGE_SIZE);
        }, nullptr, 1000);
        // Reset RAM data to initial values.
        _ramUnit.data = {};
        // Pretend we loaded from the last unit, so that the next save writes to unit zero.
        _currentUnit = STORAGE_UNITS - 1;
    }

    void save() {
        printf("> Storing data to FLASH...\n");

        // Update the CRC of the data stored in RAM.
        _ramUnit.crc = _ramUnit.compute_crc();

        // If the data we have in RAM is identical to what's stored, then don't do anything.
        if (memcmp(&_ramUnit, unit_ptr<void*>(_currentUnit), STORAGE_UNIT_SIZE) == 0) {
            printf("  No change vs already stored data\n");
            return;
        }

        // Figure out what sector we want to write to. It should be a unit that is either already
        // erased (so we can write to it as is), or a unit at the start of a sector boundary (so
        // we can erase the whole sector and then write). We start looking at the unit following the
        // current one, which guarantees that we never erase the current unit before we want to.
        int target_unit = (_currentUnit + 1) % STORAGE_UNITS;
        while (!is_erased(target_unit) && get_offset(target_unit) % FLASH_SECTOR_SIZE != 0) {
            target_unit = (target_unit + 1) % STORAGE_UNITS;
        }

        printf("  New storage unit = %d\n", target_unit);

        const auto status = flash_safe_execute([](auto param) {

            const auto unit = reinterpret_cast<intptr_t>(param);

            // Make sure we do our actual FLASH modification in a safe state.
            const auto offset = get_offset(unit);
            if (!is_erased(unit)) {
                printf("  Erase sector @ 0x%08x ...\n", offset);
                // We have already made sure that if we need to erase, it's on a sector boundary.
                assert((offset % FLASH_SECTOR_SIZE) == 0);
                flash_range_erase(offset, FLASH_SECTOR_SIZE);
            }
            // Program data to FLASH.
            printf("  Program unit %d @ 0x%08x ...\n", unit, offset);
            flash_range_program(offset, reinterpret_cast<const uint8_t *>(&_ramUnit), STORAGE_UNIT_SIZE);
            // Clear previous unit by overwriting with zeroes.
            const auto zero_data = std::unique_ptr<uint8_t[]>(new uint8_t[STORAGE_UNIT_SIZE]);
            memset(zero_data.get(), 0, STORAGE_UNIT_SIZE);
            printf("  Clear previous unit ...\n");
            flash_range_program(get_offset(_currentUnit), zero_data.get(), STORAGE_UNIT_SIZE);
            _currentUnit = unit;

        }, reinterpret_cast<void*>(target_unit), 1000);

        (void)status;
        assert(status == PICO_OK);

        printf("  Data stored to FLASH unit %d\n", _currentUnit);
    }

} // namespace storage
