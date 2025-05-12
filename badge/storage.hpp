#pragma once

#include <cstdint>
#include <span>

#include <hardware/flash.h>

namespace storage
{

    using bytes_t = std::span<const uint8_t>;

    constexpr auto STORAGE_SECTORS = 2;
    constexpr auto STORAGE_SIZE = STORAGE_SECTORS * FLASH_SECTOR_SIZE;

    enum class Key : uint8_t {
        ERASED = 0,

        ENTERED_FLAGS,

        EMPTY = 0xFF,
    };

    bytes_t read(Key key);
    void write(Key key, bytes_t data);

    void run_flash_test();

}
