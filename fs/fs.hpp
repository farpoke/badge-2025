#pragma once

#include <cstdint>
#include <span>
#include <string_view>

namespace fs
{

    std::span<const uint8_t> get_file_span(std::string_view short_name);

}
