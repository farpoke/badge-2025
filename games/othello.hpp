#pragma once

#include <array>
#include <optional>

#include <ui/state.hpp>

namespace othello
{

    enum TileState {
        TS_EMPTY = 0,

        TS_BLACK,
        TS_FLIP_B2W_1,
        TS_FLIP_B2W_2,
        TS_FLIP_B2W_3,

        TS_WHITE,
        TS_FLIP_W2B_1,
        TS_FLIP_W2B_2,
        TS_FLIP_W2B_3,
    };

    enum GameState {
        GS_WAITING_ON_PLAYER,
        GS_ANIMATING_FLIPS,
        GS_ILLEGAL_MOVE,
        GS_GAME_OVER,
    };

    struct BoardState {
        std::array<std::array<TileState, 8>, 8> board = {};
        bool black_plays_next = true;
        int black_pieces = 0;
        int white_pieces = 0;

        std::array<std::array<int, 8>, 8> move_flips = {};

        static BoardState initial_state();

        void update_move_flips();
        int count_flips(int r, int c) const;
        bool make_play(int r, int c);

    private:
        int count_flips(int r0, int c0, int dr, int dc) const;
        int start_flips(int r0, int c0, int dr, int dc);

    };

    class OthelloGame final : public ui::State {
    public:
        void update(int delta_ms) override;
        void draw() override;

        void pause() override;
        void resume() override;

    private:
        GameState state = GS_WAITING_ON_PLAYER;
        BoardState board = {};
        int animation_timer = 0;
        int animation_counter = 0;
        int cursor_row = 0;
        int cursor_col = 0;

        void reset();
        void attempt_play();
        void animate_board();
    };

} // namespace othello
