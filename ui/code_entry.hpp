#pragma once

#include "state.hpp"

#include <string>
#include <vector>

#include <badge/flags.hpp>

namespace ui
{

    struct Button;

    class CodeEntry final : public State {
    public:

        enum Layout : int {
            EL_UPPERCASE = 1,
            EL_LOWERCASE = 2,
            EL_DIGITS = 3,
            EL_OTHER = 4,
        };

        void update(int delta_ms) override;
        void draw() override;

        void pause() override;
        void resume() override;

    protected:

        Layout current_layout = EL_UPPERCASE;
        std::vector<Button*> buttons;
        Button* selected_button = nullptr;
        int press_timer = 0;
        int konami_count = 0;
        bool show_konami = false;
        int show_flag_timer = 0;
        flags::Flag flag = flags::INVALID;

        std::string entry_text;

        void append_char(char ch);
        void delete_char();

        void enter();

    };

}
