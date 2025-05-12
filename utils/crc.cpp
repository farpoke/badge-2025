#include "crc.hpp"

#include <array>

namespace utils
{

    constexpr uint8_t CRC8_POLYNOMIAL = 0x8C; // Reverse of "standard" polynomial 0x31

    consteval auto compute_crc8_table() {
        std::array<uint8_t, 256> result = {};
        for (int idx = 0; idx < 256; idx++) {
            uint8_t crc = idx;
            for (int bit = 0; bit < 8; bit++) {
                crc = (crc >> 1) ^ (crc & 1 ? CRC8_POLYNOMIAL : 0);
            }
            result[idx] = crc;
        }
        return result;
    }

    constexpr auto CRC8_TABLE = compute_crc8_table();

    constexpr auto compute_crc8(std::span<const uint8_t> bytes) {
        uint8_t crc = 0;
        for (auto byte : bytes)
            crc = CRC8_TABLE[crc ^ byte];
        return crc;
    }

    constexpr std::array<uint8_t, 9> TEST_DATA = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
    static_assert(compute_crc8(TEST_DATA) == 0xA1);

}
