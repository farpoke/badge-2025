#pragma once

#include <cstdint>

namespace core1
{

    constexpr uint32_t CORE1_STACK_SIZE = 0x800;

    void reset_and_launch();
    void swap_frame();

}
