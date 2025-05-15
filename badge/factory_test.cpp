#include "factory_test.hpp"

#include <array>

#include <badge/buttons.hpp>
#include <badge/drawing.hpp>
#include <badge/storage.hpp>

namespace factory
{

    static constexpr std::array ITEM_XY = {
        std::pair{41, 43},
        std::pair{41, 71},
        std::pair{27, 57},
        std::pair{55, 57},
        std::pair{41, 57},
        std::pair{104, 71},
        std::pair{118, 57},
        std::pair{90, 57},
        std::pair{104, 43},
        std::pair{73, 95},
    };

    static uint32_t get_button_bits() {
        uint32_t value = 0;
#define BTN(NAME) if (buttons::get_current(1 << BTN_##NAME)) value |= 1 << TestItems::NAME;
        BTN(UP)
        BTN(DOWN)
        BTN(LEFT)
        BTN(RIGHT)
        BTN(PUSH)
        BTN(A)
        BTN(B)
        BTN(C)
        BTN(D)
#undef BTN
        return value;
    }

    void FactoryTest::update(int delta_ms) {
        storage::ram_data->factory_test_result |= get_button_bits();
        const bool all = (storage::ram_data->factory_test_result & ALL_ITEMS_MASK) == ALL_ITEMS_MASK;
        if (!stored && all) {
            storage::save();
            stored = true;
        }
    }

    void FactoryTest::draw() {
        drawing::clear(COLOR_BLACK);

        const auto pressed = get_button_bits();
        const auto ok = storage::ram_data->factory_test_result;

        for (int i = 0; i < N_ITEMS; i++) {
            Pixel color = COLOR_RED;
            if (pressed & (1 << i))
                color = COLOR_WHITE;
            else if (ok & (1 << i))
                color = COLOR_GREEN;
            const auto& [x, y] = ITEM_XY[i];
            drawing::fill_rect(x, y, 12, 12, color);
        }

        const auto& [x, y] = ITEM_XY[N_ITEMS];
        drawing::fill_rect(x, y, 12, 12, stored ? COLOR_GREEN : COLOR_RED);
    }

    void FactoryTest::resume() {
        stored = (storage::ram_data->factory_test_result & ALL_ITEMS_MASK) == ALL_ITEMS_MASK;
    }

}
