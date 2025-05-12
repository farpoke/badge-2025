#include "storage.hpp"

#include <cstdio>
#include <cstring>
#include <memory>

#include <hardware/flash.h>
#include <pico/flash.h>

#include <badge/badge-2025.h>

namespace storage
{

    constexpr intptr_t STORAGE_END = XIP_BASE + BADGE_FLASH_SIZE;
    constexpr intptr_t STORAGE_BEGIN = STORAGE_END - STORAGE_SIZE;

    static const uint8_t &at(size_t offset) {
        assert(offset >= 0 && offset < STORAGE_SIZE);
        return *reinterpret_cast<const uint8_t *>(STORAGE_BEGIN + offset);
    }

    bytes_t read(Key key) {
        size_t offset = 0;
        while (offset < STORAGE_SIZE && at(offset) == 0)
            offset++;
        while (offset + 2 < STORAGE_SIZE && at(offset) != 0xFF) {
            const auto current_key = static_cast<Key>(at(offset++));
            const auto length = at(offset++);
            if (current_key == key) {
                assert(offset + length <= STORAGE_SIZE);
                return bytes_t(&at(offset), &at(offset + length));
            }
            else {
                offset += length;
            }
        }
        return {};
    }

    void write(Key key, bytes_t data) {}

    bool compare_flash(const uint8_t *expected) {
        for (size_t i = 0; i < STORAGE_SIZE; i++)
            if (at(i) != expected[i])
                return false;
        return true;
    }

    void run_flash_test() {

        printf("\n----- FLASH TEST -----\n");

        auto buffer = std::unique_ptr<uint8_t[]>(new uint8_t[STORAGE_SIZE]);

#define DO_CHECK                                                                                                       \
    if (compare_flash(buffer.get()))                                                                                   \
        printf("OK\n");                                                                                                \
    else {                                                                                                             \
        printf("FAIL\n\n");                                                                                            \
        return;                                                                                                        \
    }

#define PAGE_PROGRAM_AND_CHECK                                                                                              \
    flash_safe_execute(                                                                                                \
            [](auto data) {                                                                                            \
                flash_range_program(BADGE_FLASH_SIZE - STORAGE_SIZE, static_cast<const uint8_t *>(data), 256);         \
            },                                                                                                         \
            buffer.get(),                                                                                              \
            500);                                                                                                      \
    DO_CHECK

        printf("> Erase storage flash... ");
        flash_safe_execute([](auto) { flash_range_erase(BADGE_FLASH_SIZE - STORAGE_SIZE, STORAGE_SIZE); },
                           nullptr,
                           500);
        memset(buffer.get(), 0xFF, STORAGE_SIZE);
        DO_CHECK

        for (int i = 0; i < 200; i++)
            buffer[i] = i;

        printf("> Program page... ");
        PAGE_PROGRAM_AND_CHECK

        printf("> Re-program page... ");
        PAGE_PROGRAM_AND_CHECK

        for (int i = 0; i < 100; i++)
            buffer[i] = 0;

        printf("> Partial zero... ");
        PAGE_PROGRAM_AND_CHECK

        printf("\n");
    }

} // namespace storage
