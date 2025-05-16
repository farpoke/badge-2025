#include "flag_view.hpp"

#include <badge/buttons.hpp>
#include <badge/drawing.hpp>
#include <badge/flags.hpp>
#include <badge/lcd.hpp>
#include <ui/ui.hpp>

namespace ui
{

    namespace
    {
        constexpr auto IMAGE_SIZE = 24;
        constexpr auto SPACING = 2;
        constexpr auto STRIDE = IMAGE_SIZE + SPACING;
        constexpr auto COLUMNS = (lcd::WIDTH - SPACING) / STRIDE;
        constexpr auto X0 = (lcd::WIDTH + SPACING - COLUMNS * STRIDE) / 2;
    } // namespace

    void FlagView::update(int delta_ms) {
        if (buttons::up()) {
            scroll -= delta_ms / 2;
            if (scroll < 0)
                scroll = 0;
        }
        else if (buttons::down()) {
            scroll += delta_ms / 2;
            if (scroll > max_scroll)
                scroll = max_scroll;
        }
        else if (buttons::b()) {
            pop_state();
        }
    }

    void FlagView::draw() {
        drawing::clear(COLOR_BLACK);
        if (items.empty()) {
            drawing::draw_text_centered(lcd::WIDTH / 2,
                                        lcd::HEIGHT / 2,
                                        "No flags entered yet",
                                        COLOR_WHITE,
                                        font::m6x11);
            return;
        }
        for (const auto &[image, left, top] : items) {
            drawing::draw_image(left, top - scroll, *image);
        }
    }

    void FlagView::pause() { items.clear(); }

    void FlagView::resume() {
        int x = X0;
        int y = SPACING;
        for (auto flag : flags::get_found_flags()) {
            const Item item{&flags::get_flag_image(flag), x, y};
            items.push_back(item);
            x += STRIDE;
            if (x + STRIDE >= lcd::WIDTH) {
                x = X0;
                y += STRIDE;
            }
        }
        if (y + STRIDE > lcd::HEIGHT) {
            max_scroll = y + STRIDE - lcd::HEIGHT;
        }
        else {
            max_scroll = 0;
        }
        scroll = 0;
    }


} // namespace ui
