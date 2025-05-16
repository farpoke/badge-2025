#include "menu.hpp"

#include <assets.hpp>
#include <badge/buttons.hpp>
#include <badge/drawing.hpp>
#include <badge/font.hpp>

#include "ui.hpp"

namespace ui
{

    void Menu::add_item(std::string_view label, const StatePtr &target_state) {
        items.emplace_back(label, target_state);
    }

    void Menu::add_item(std::string_view label, Callback callback) {
        items.emplace_back(label, callback);
    }

    void Menu::update(int delta_ms) {
        State::update(delta_ms);

        if (target_offset == 0) {
            // Do nothing.
        }
        else if (abs(target_offset) < 2) {
            current_offset += target_offset;
            target_offset = 0;
        }
        else {
            current_offset += target_offset * delta_ms / 100;
        }

        if (items.empty()) return;

        if (buttons::a() || buttons::push()) {
            const auto& item = items[selected_item];
            if (item.target_state)
                push_state(item.target_state);
            else if (item.callback)
                item.callback();
        }
        else if (buttons::down()) {
            selected_item = (selected_item + 1) % items.size();
        }
        else if (buttons::up()) {
            selected_item = (selected_item + items.size() - 1) % items.size();
        }
        else if (buttons::b() && !is_main) {
            pop_state();
        }
    }

    void Menu::draw() {
        drawing::clear(COLOR_BLACK);
        // drawing::draw_image(10, 10, image::triangle_right);

        int y0 = lcd::HEIGHT / 2 + current_offset;

        constexpr auto selected_color = COLOR_WHITE;
        constexpr auto other_color = rgb888(100, 100, 100);

        for (int i = 0; i < static_cast<int>(items.size()); i++) {
            const auto& item = items[i];
            auto render = font::m6x11.render(item.label);
            const auto x = lcd::WIDTH / 2 - render.width / 2;
            const auto y = y0 - render.height / 2;
            const auto color = (i == selected_item) ? selected_color : other_color;
            drawing::draw_text(x - render.dx, y - render.dy, 0, 0, color, render);
            if (i == selected_item) {
                target_offset = lcd::HEIGHT / 2 - y0;
                drawing::draw_image(x - 16, y0 - 6, image::triangle_right);
                drawing::draw_image(x + render.width + 3, y0 - 6, image::triangle_left);
            }
            y0 += render.height + 2;
        }
    }

}
