#include "splash.hpp"
#include "ui.hpp"

#include <assets.hpp>
#include <badge/drawing.hpp>

namespace ui
{

    void SplashScreen::update(int delta_ms) {
        State::update(delta_ms);
        if (time_ms >= DURATION_MS)
            pop_state();
    }

    void SplashScreen::draw() {
        drawing::clear(lcd::to_pixel(20, 40, 60));
        drawing::draw_image(0, 0, image::splash_bg);
        drawing::fill_masked(0, 0, lcd::from_grayscale(0), image::splash_fg);
        drawing::fill_rect(20, 20, 81, 81, lcd::to_pixel(20, 30, 40));
        drawing::draw_rect(25, 25, 71, 71, lcd::to_pixel(255, 128, 0));
        constexpr auto line_color = lcd::to_pixel(0, 255, 255);
        for (int i = 0; i < 70; i += 10) {
            auto offset = (i + time_ms / 20) % 70;
            drawing::draw_line_aa(60, 60, 25, 25 + offset, line_color);
            drawing::draw_line_aa(60, 60, 95, 95 - offset, line_color);
            drawing::draw_line_aa(60, 60, 95 - offset, 25, line_color);
            drawing::draw_line_aa(60, 60, 25 + offset, 95, line_color);
        }
    }

}
