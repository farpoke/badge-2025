#pragma once

#include "state.hpp"

#include <string_view>
#include <vector>

#include <badge/font.hpp>

namespace ui
{

    class Readme final : public State {
    public:

        void update(int delta_ms) override;
        void draw() override;

        void pause() override;
        void resume() override;

    protected:
        const font::Font* font = nullptr;
        int line_height = 0;
        std::vector<std::string_view> lines = {};
        int padding = 2;
        int scroll = 0;
        int max_scroll = 0;
        bool is_scrolling = false;

    };

}
