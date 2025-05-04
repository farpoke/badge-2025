#include "splash.hpp"
#include "ui.hpp"

#include <cstring>

#include <assets.hpp>
#include <badge/drawing.hpp>

namespace ui
{

    SplashScreen::SplashScreen() {
        mask = new uint8_t[lcd::WIDTH * lcd::HEIGHT];
        memset(mask, 0, lcd::WIDTH * lcd::HEIGHT);

        bg_image = image::splash_bg;
        bg_image.alpha_data = mask;
    }

    SplashScreen::~SplashScreen() {
        delete[] mask;
    }

    void SplashScreen::update(int delta_ms) {
        while (delta_ms > 0) {
            delta_ms--;
            time_ms++;

            if (time_ms % TICK_DIVIDER != 0)
                continue;

            int diagonal = time_ms / TICK_DIVIDER - 1;

            if (diagonal <= lcd::WIDTH + lcd::HEIGHT) {
                for (int y = 0; y < lcd::HEIGHT; y++) {
                    const int x = diagonal - y;
                    if (x >= 0 && x < lcd::WIDTH)
                        mask[y * lcd::WIDTH + x] = 255 - image::splash_fg.alpha_data[y * lcd::WIDTH + x];
                }
            }

            diagonal -= DELAY_1;

            if (diagonal >= 0 && diagonal <= lcd::WIDTH + lcd::HEIGHT) {
                for (int y = 0; y < lcd::HEIGHT; y++) {
                    const int x = diagonal - y;
                    if (x >= 0 && x < lcd::WIDTH)
                        mask[y * lcd::WIDTH + x] = image::splash_fg.alpha_data[y * lcd::WIDTH + x];
                }
            }

            diagonal -= DELAY_2;

            if (diagonal >= 0 && diagonal <= lcd::WIDTH + lcd::HEIGHT) {
                for (int y = 0; y < lcd::HEIGHT; y++) {
                    const int x = diagonal - y;
                    if (x >= 0 && x < lcd::WIDTH)
                        mask[y * lcd::WIDTH + x] = 0;
                }
            }
        }

        if (time_ms > DURATION_MS)
            pop_state();
    }

    void SplashScreen::draw() {
        drawing::clear(0);
        drawing::draw_image(0, 0, bg_image);
    }

} // namespace ui
