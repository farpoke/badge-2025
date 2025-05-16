#pragma once

#include <cstdint>
#include <span>

#include <hardware/flash.h>

#include <badge/flags.hpp>

namespace storage
{

    constexpr auto STORAGE_UNIT_PAGES = 4;
    constexpr auto STORAGE_UNIT_SIZE = FLASH_PAGE_SIZE * STORAGE_UNIT_PAGES;

    static_assert(FLASH_SECTOR_SIZE % STORAGE_UNIT_SIZE == 0);

    constexpr auto FLAG_STORAGE_SIZE = 500;

    struct StorageData {

        uint32_t factory_test_result = 0;

        int snek_highscore = 0;
        int blocks_highscore = 0;

        int reserved[29] = {};

        char entered_flags[FLAG_STORAGE_SIZE] = {};

    };

    static_assert(sizeof(StorageData) < STORAGE_UNIT_SIZE - 4);

    extern StorageData* ram_data;

    void init();
    void erase();
    void save();

}
