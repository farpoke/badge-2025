#pragma once

#include <cstdint>
#include <span>

namespace utils
{

    uint8_t crc8(std::span<const std::byte> data);

}
