#pragma once

#include <ui/state.hpp>

namespace factory
{

    enum TestItems {
        UP = 0,
        DOWN,
        LEFT,
        RIGHT,
        PUSH,
        A,
        B,
        C,
        D,

        N_ITEMS
    };

    constexpr uint32_t ALL_ITEMS_MASK = (1 << N_ITEMS) - 1;

    class FactoryTest final : public ui::State {
    public:
        void update(int delta_ms) override;
        void draw() override;

        void resume() override;

    private:
        bool stored = false;
    };

} // namespace factory
