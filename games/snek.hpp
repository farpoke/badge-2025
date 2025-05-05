#pragma once

#include <ui/state.hpp>

namespace snek
{

    enum class GameState {
        WAITING_TO_START,
        PLAYING,
        DEAD,
        AFTERLIFE,
    };

    enum class CellState : uint8_t {
        EMPTY = 0,
        HEAD,
        FRUIT,
        LEFT,
        RIGHT,
        UP,
        DOWN,
    };

    class SnekGame final : public ui::State {
    public:
        void update(int delta_ms) override;
        void draw() override;

        void pause() override;
        void resume() override;

    private:
        GameState game_state = {};

        CellState *grid = nullptr;

        int head_u  = 0;
        int head_v  = 0;
        int tail_u  = 0;
        int tail_v  = 0;
        int length  = 0;
        int fruit_u = 0;
        int fruit_v = 0;

        CellState direction = {};

        int move_timer = 0;
        int move_interval = 1000;

        int score = 0;

        bool changed_direction = false;
        CellState queued_direction = {};

        CellState &at(int u, int v);

        void reset();
        void advance();
        void steer(CellState dir);
        void place_fruit();

    };

} // namespace snek
