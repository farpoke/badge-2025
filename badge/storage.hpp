#pragma once

#include <cstdint>
#include <span>

#include <hardware/flash.h>

#include <badge/flags.hpp>

namespace storage
{

    constexpr auto STORAGE_UNIT_PAGES = 2;
    constexpr auto STORAGE_UNIT_SIZE = FLASH_PAGE_SIZE * STORAGE_UNIT_PAGES;

    static_assert(FLASH_SECTOR_SIZE % STORAGE_UNIT_SIZE == 0);

    struct StorageData {

        char entered_flags[flags::FLAG_COUNT][flags::MAX_FLAG_LENGTH] = {};

        int snek_highscore = 0;
        int blocks_highscore = 0;

    };

    static_assert(sizeof(StorageData) < STORAGE_UNIT_SIZE - 4);

    extern StorageData* ram_data;

    void init();
    void erase();
    void save();

}
