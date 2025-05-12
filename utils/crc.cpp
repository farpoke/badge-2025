#include "crc.hpp"

#include <array>

namespace utils
{

    // ========== CRC-8 ========== //

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

    uint8_t crc8(std::span<const uint8_t> data) { return compute_crc8(data); }



    // ========== CRC-32 ========== //

    constexpr uint32_t CRC32_POLYNOMIAL = 0xEDB88320; // Reverse of "standard" polynomial 0x04C11DB7

    consteval auto compute_crc32_table() {
        std::array<uint32_t, 256> result = {};
        for (int idx = 0; idx < 256; idx++) {
            uint32_t crc = idx;
            for (int bit = 0; bit < 8; bit++) {
                crc = (crc >> 1) ^ (crc & 1 ? CRC32_POLYNOMIAL : 0);
            }
            result[idx] = crc;
        }
        return result;
    }

    constexpr auto CRC32_TABLE = compute_crc32_table();

    constexpr auto compute_crc32(std::span<const uint8_t> bytes) {
        uint32_t crc = ~0;
        for (auto byte : bytes)
            crc = CRC32_TABLE[(crc ^ byte) & 0xFF] ^ (crc >> 8);
        return ~crc;
    }

    static_assert(compute_crc32(TEST_DATA) == 0xCBF43926);

    uint32_t crc32(std::span<const uint8_t> data) { return compute_crc32(data); }

}
