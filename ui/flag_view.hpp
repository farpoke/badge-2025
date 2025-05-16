#pragma once

#include <vector>

#include <badge/image.hpp>

#include <ui/state.hpp>

namespace ui
{

    class FlagView final : public State {
    public:

        void update(int delta_ms) override;
        void draw() override;

        void pause() override;
        void resume() override;

    private:
        struct Item {
            const image::Image* image;
            int left;
            int top;
        };

        std::vector<Item> items = {};
        int scroll = 0;
        int max_scroll = 0;

    };

}
