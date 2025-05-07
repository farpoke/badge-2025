#pragma once

#include <array>

#include <ui/state.hpp>

namespace othello
{

    enum TileState {
        TS_EMPTY = 0,
        TS_WHITE,
        TS_BLACK,

        TS_FLIP_W2B_1,
        TS_FLIP_W2B_2,
        TS_FLIP_W2B_3,

        TS_FLIP_B2W_1,
        TS_FLIP_B2W_2,
        TS_FLIP_B2W_3,
    };

    enum GameState {
        GS_WAITING_ON_PLAYER,
        GS_ANIMATING,
        GS_GAME_OVER,
    };

    class OthelloGame final : public ui::State {
    public:
        void update(int delta_ms) override;
        void draw() override;

        void pause() override;
        void resume() override;

    private:
        std::array<std::array<TileState, 8>, 8> board = {};
        GameState state = GS_WAITING_ON_PLAYER;
        bool white_plays_next = false;
        int animation_timer = 0;

        void reset();

        void animate_board();
    };

} // namespace othello
