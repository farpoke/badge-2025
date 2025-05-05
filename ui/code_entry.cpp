#include "code_entry.hpp"

#include <cassert>

#include <badge/buttons.hpp>
#include <badge/drawing.hpp>
#include <cstring>
#include <ui/ui.hpp>

namespace ui
{

    using enum CodeEntry::Layout;

    static constexpr auto BUTTON_WIDTH = 16;
    static constexpr auto BUTTON_HEIGHT = 20;

    static constexpr auto KEYBOARD_X0 = 3;
    static constexpr auto KEYBOARD_Y0 = lcd::HEIGHT - (BUTTON_HEIGHT - 1) * 4 - 1;

    static constexpr auto NORMAL_BG = COLOR_BLACK;
    static constexpr auto NORMAL_FG = COLOR_WHITE;

    static constexpr auto SELECTED_BG = rgb24(0xFF8800);
    static constexpr auto SELECTED_FG = COLOR_BLACK;

    static constexpr auto ACTIVE_BG = COLOR_WHITE;
    static constexpr auto ACTIVE_FG = COLOR_BLACK;

    static constexpr auto BORDER_COLOR = COLOR_WHITE;

    static constexpr const char* LAYOUT_UPPERCASE[3] = {
        "ABCDEFGHIJ",
        "KLMNOPQRST",
        "UVWXYZ",
    };

    static constexpr const char* LAYOUT_LOWERCASE[3] = {
        "abcdefghij",
        "klmnopqrst",
        "uvwxyz",
    };

    static constexpr const char* LAYOUT_DIGITS[3] = {
        "1234567890",
        "@#$&_-()=%",
        ".,:;!?",
    };

    static constexpr const char* LAYOUT_OTHERS[3] = {
        "1234567890",
        "[]{}<>^   ",
        "~+/\\; ",
    };

    enum SpecialValue : char {
        ESV_LAYOUT_SWITCH_1 = 1,
        ESV_LAYOUT_SWITCH_2 = 2,
        ESV_EXIT = 3,
        ESV_SPACE = 4,
        ESV_BACKSPACE = 5,
        ESV_ENTER = 6,
    };

    static constexpr auto SWITCH_CASE_LABEL    = "A/a";
    static constexpr auto SWITCH_LETTERS_LABEL = "ABC";
    static constexpr auto SWITCH_NUMBERS_LABEL = "123";
    static constexpr auto SWITCH_SYMBOLS_LABEL = "<[{";

    static constexpr auto EXIT_LABEL = "Exit";
    static constexpr auto SPACE_LABEL = " ";
    static constexpr auto BACKSPACE_LABEL = "<-";
    static constexpr auto ENTER_LABEL = "OK";

    struct Button {
        int x = 0, y = 0;
        int w = 0, h = 0;
        int row = 0;
        int col = 0;
        int col_span = 1;
        char value = 0;
        std::string_view label = {};
        Button* up = nullptr;
        Button* down = nullptr;
        Button* left = nullptr;
        Button* right = nullptr;
    };

    void build_keyboard(std::vector<Button*>& button_collection) {

        // Use this temporary array to keep track of buttons and link them together appropriately.
        Button* grid[4][10];
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 10; j++)
                grid[i][j] = nullptr;

        const auto create_button = [&](int row, int col, int span_cols) {
            auto* new_button = new Button();

            new_button->x = KEYBOARD_X0 + col * (BUTTON_WIDTH - 1);
            new_button->y = KEYBOARD_Y0 + row * (BUTTON_HEIGHT - 1);
            new_button->w = 1 + span_cols * (BUTTON_WIDTH - 1);
            new_button->h = BUTTON_HEIGHT;
            new_button->row = row;
            new_button->col = col;
            new_button->col_span = span_cols;
            new_button->value = ' ';
            new_button->label = {};

            for (int i = col; i < col + span_cols; i++) {
                assert(grid[row][i] == nullptr);
                grid[row][i] = new_button;
            }

            button_collection.push_back(new_button);

            return new_button;
        };

        // Create the "normal" buttons.
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 10; col++) {
                create_button(row, col, 1);

                // Exit out early from the last row (only 6 buttons instead of 10).
                if (row == 2 && col == 5)
                    break;
            }
        }

        // Create the two layout-changing buttons to fill out the third row.
        // They each span two columns, and get special values.
        create_button(2, 6, 2)->value = ESV_LAYOUT_SWITCH_1;
        create_button(2, 8, 2)->value = ESV_LAYOUT_SWITCH_2;

        // Create the special buttons of the fourth and final row.
        create_button(3, 0, 2)->value = ESV_EXIT;
        create_button(3, 2, 4)->value = ESV_SPACE;
        create_button(3, 6, 2)->value = ESV_BACKSPACE;
        create_button(3, 8, 2)->value = ESV_ENTER;

        // Now that we have all the buttons, use the temporary grid to link them together for navigation.
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 10; col++) {
                auto* button = grid[row][col];
                assert(button != nullptr);
                if (button->up == nullptr)
                    button->up = grid[(button->row + 3) % 4][button->col];
                if (button->down == nullptr)
                    button->down = grid[(button->row + 1) % 4][button->col];
                if (button->left == nullptr)
                    button->left = grid[button->row][(button->col + 9) % 10];
                if (button->right == nullptr)
                    button->right = grid[button->row][(button->col + button->col_span) % 10];
            }
        }
    }

    void update_keyboard(const std::vector<Button*>& button_collection, CodeEntry::Layout active_layout) {
        const char* const* character_layout;
        if (active_layout == EL_UPPERCASE) character_layout = LAYOUT_UPPERCASE;
        else if (active_layout == EL_LOWERCASE) character_layout = LAYOUT_LOWERCASE;
        else if (active_layout == EL_DIGITS) character_layout = LAYOUT_DIGITS;
        else character_layout = LAYOUT_OTHERS;

        for (auto* button : button_collection) {
            if (button->value >= ' ') {
                button->value = character_layout[button->row][button->col];
                button->label = std::string_view(&button->value, 1);
            }
            else if (button->value == ESV_LAYOUT_SWITCH_1) {
                if (active_layout == EL_UPPERCASE || active_layout == EL_LOWERCASE)
                    button->label = SWITCH_CASE_LABEL;
                else if (active_layout == EL_DIGITS)
                    button->label = SWITCH_SYMBOLS_LABEL;
                else if (active_layout == EL_OTHER)
                    button->label = SWITCH_NUMBERS_LABEL;
            }
            else if (button->value == ESV_LAYOUT_SWITCH_2) {
                if (active_layout == EL_UPPERCASE || active_layout == EL_LOWERCASE)
                    button->label = SWITCH_NUMBERS_LABEL;
                else
                    button->label = SWITCH_LETTERS_LABEL;
            }
            else if (button->value == ESV_EXIT) {
                button->label = EXIT_LABEL;
            }
            else if (button->value == ESV_SPACE) {
                button->label = SPACE_LABEL;
            }
            else if (button->value == ESV_BACKSPACE) {
                button->label = BACKSPACE_LABEL;
            }
            else if (button->value == ESV_ENTER) {
                button->label = ENTER_LABEL;
            }
            else {
                button->label = "???";
            }
        }
    }

    void CodeEntry::update(int delta_ms) {
        State::update(delta_ms);

        if (press_timer > 0) {
            press_timer -= delta_ms;
            if (press_timer < 0)
                press_timer = 0;
        }

        if (buttons::a() || buttons::push()) {
            const auto value = selected_button->value;
            press_timer = 100;
            if (value == ESV_LAYOUT_SWITCH_1) {
                if (current_layout == EL_UPPERCASE) current_layout = EL_LOWERCASE;
                else if (current_layout == EL_LOWERCASE) current_layout = EL_UPPERCASE;
                else if (current_layout == EL_DIGITS) current_layout = EL_OTHER;
                else current_layout = EL_DIGITS;
                update_keyboard(buttons, current_layout);
            }
            else if (value == ESV_LAYOUT_SWITCH_2) {
                if (current_layout == EL_UPPERCASE || current_layout == EL_LOWERCASE)
                    current_layout = EL_DIGITS;
                else
                    current_layout = EL_UPPERCASE;
                update_keyboard(buttons, current_layout);
            }
            else if (value == ESV_EXIT) {
                pop_state();
            }
            else if (value == ESV_SPACE) {
                append_char(' ');
            }
            else if (value == ESV_BACKSPACE) {
                delete_char();
            }
            else if (value == ESV_ENTER) {
                // ...
            }
            else if (value > ' ') {
                append_char(value);
            }
        }
        else if (buttons::b())
            delete_char();
        else if (buttons::up())
            selected_button = selected_button->up;
        else if (buttons::down())
            selected_button = selected_button->down;
        else if (buttons::left())
            selected_button = selected_button->left;
        else if (buttons::right())
            selected_button = selected_button->right;
    }

    void CodeEntry::draw() {
        drawing::clear(COLOR_BLACK);

        for (const auto* button : buttons) {
            auto bg_color = NORMAL_BG;
            auto fg_color = NORMAL_FG;
            if (button == selected_button) {
                if (press_timer > 0) {
                    bg_color = ACTIVE_BG;
                    fg_color = ACTIVE_FG;
                }
                else {
                    bg_color = SELECTED_BG;
                    fg_color = SELECTED_FG;
                }
            }
            drawing::fill_rect(button->x, button->y, button->w, button->h, bg_color);
            drawing::draw_rect(button->x, button->y, button->w, button->h, BORDER_COLOR);
            const auto render = font::m6x11.render(button->label);
            const int x = button->x + button->w / 2 - render.dx - render.width / 2;
            const int y = button->y + button->h / 2 - render.dy - render.height / 2;
            drawing::draw_text(x, y, 0, 0, fg_color, render);
        }

        const auto render = font::m6x11.render("gbgay{" + entry_text + "}");
        const int x = lcd::WIDTH / 2 - render.dx - render.width / 2;
        const int y = KEYBOARD_Y0 / 2 - render.dy - render.height / 2;
        drawing::draw_text(x, y, 0, 0, COLOR_WHITE, render);
    }

    void CodeEntry::pause() {
        for (const auto button : buttons) {
            delete button;
        }
        buttons = {};
    }

    void CodeEntry::resume() {
        build_keyboard(buttons);
        current_layout = EL_UPPERCASE;
        update_keyboard(buttons, current_layout);
        selected_button = buttons.front();
        press_timer = 0;
        entry_text = {};
    }

    void CodeEntry::append_char(char ch) {
        entry_text += ch;
    }

    void CodeEntry::delete_char() {
        if (!entry_text.empty())
            entry_text.erase(entry_text.end() - 1);
    }


}
