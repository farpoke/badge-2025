#pragma once

#include <cstdint>
#include <span>

namespace utils
{

    uint8_t crc8(std::span<const uint8_t> data);

    uint32_t crc32(std::span<const uint8_t> data);

}
