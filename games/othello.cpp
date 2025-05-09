#include "othello.hpp"

#include <cassert>

#include <assets.hpp>
#include <badge/drawing.hpp>
#include <badge/pixel.hpp>

#include "badge/buttons.hpp"
#include "ui/ui.hpp"

namespace othello
{

    constexpr auto BG_COLOR = rgb888(10, 50, 10);
    constexpr auto BORDER_COLOR = COLOR_WHITE;
    constexpr auto DIVIDER_COLOR = COLOR_BLACK;

    constexpr auto TILE_SIZE = 15;
    constexpr auto BOARD_SIZE = TILE_SIZE * 8 - 1;
    constexpr auto BOARD_LEFT = (lcd::WIDTH - BOARD_SIZE) / 2;
    constexpr auto BOARD_TOP = (lcd::HEIGHT - BOARD_SIZE) / 2;

    constexpr auto PIECE_OFFSET = 1;

    constexpr auto FLIP_ANIMATION_INTERVAL = 100;
    constexpr auto CURSOR_ANIMATION_INTERVAL = 200;

    const image::Image *get_tile_image(TileState tile) {
        switch (tile) {
        case TS_WHITE: return &image::othello_white;
        case TS_BLACK: return &image::othello_black;
        case TS_FLIP_W2B_1:
        case TS_FLIP_B2W_3: return &image::othello_flip1;
        case TS_FLIP_W2B_2:
        case TS_FLIP_B2W_2: return &image::othello_flip2;
        case TS_FLIP_W2B_3:
        case TS_FLIP_B2W_1: return &image::othello_flip3;
        default: return nullptr;
        }
    }

    TileState get_next_state(TileState tile) {
        if (tile > TS_EMPTY && tile < TS_WRAP_STATE - 1)
            return static_cast<TileState>(tile + 1);
        else if (tile == TS_WRAP_STATE - 1)
            return static_cast<TileState>(TS_EMPTY + 1);
        else
            return tile;
    }

    BoardState BoardState::initial_state() {
        BoardState result;

        // Clear the board.
        for (auto &row : result.board)
            for (auto &tile : row)
                tile = TS_EMPTY;

        // Add in the fixed starting pieces.
        result.board[3][3] = result.board[4][4] = TS_WHITE;
        result.board[3][4] = result.board[4][3] = TS_BLACK;

        // Set initial info.
        result.black_plays_next = true;
        result.black_pieces = 2;
        result.white_pieces = 2;

        result.update_move_flips();

        return result;
    }

    void BoardState::update_move_flips() {
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                move_flips[r][c] = count_flips(r, c);
            }
        }
    }

    int BoardState::count_flips(int r, int c) const {
        if (board[r][c] != TS_EMPTY)
            return 0;
        int sum = 0;
        sum += count_flips(r, c, 1, 0);
        sum += count_flips(r, c, -1, 0);
        sum += count_flips(r, c, 0, 1);
        sum += count_flips(r, c, 0, -1);
        sum += count_flips(r, c, 1, 1);
        sum += count_flips(r, c, -1, 1);
        sum += count_flips(r, c, -1, -1);
        sum += count_flips(r, c, 1, -1);
        return sum;
    }

    bool BoardState::make_play(int r, int c) {
        if (board[r][c] != TS_EMPTY)
            return false;
        int sum = 0;
        sum += start_flips(r, c, 1, 0);
        sum += start_flips(r, c, -1, 0);
        sum += start_flips(r, c, 0, 1);
        sum += start_flips(r, c, 0, -1);
        sum += start_flips(r, c, 1, 1);
        sum += start_flips(r, c, -1, 1);
        sum += start_flips(r, c, -1, -1);
        sum += start_flips(r, c, 1, -1);
        if (sum == 0)
            return false;
        if (black_plays_next) {
            board[r][c] = TS_BLACK;
            black_pieces += sum + 1;
            white_pieces -= sum;
            black_plays_next = false;
        }
        else {
            board[r][c] = TS_WHITE;
            white_pieces += sum + 1;
            black_plays_next -= sum;
            black_plays_next = true;
        }
        return true;
    }

    int BoardState::count_flips(int r0, int c0, int dr, int dc) const {
        const auto player = black_plays_next ? TS_BLACK : TS_WHITE;
        const auto opponent = black_plays_next ? TS_WHITE : TS_BLACK;
        int r = r0;
        int c = c0;
        for (int i = 1; i < 8; i++) {
            r += dr;
            c += dc;
            if (r < 0 || r >= 8 || c < 0 || c >= 8)
                return 0;
            if (board[r][c] == player)
                return i - 1;
            else if (board[r][c] == opponent)
                continue;
            else
                return 0;
        }
        assert(false && "Logic error in othello::BoardState::count_flips");
    }

    int BoardState::start_flips(int r0, int c0, int dr, int dc) {
        const int n = count_flips(r0, c0, dr, dc);
        for (int i = 1; i <= n; i++) {
            const int r = r0 + dr * i;
            const int c = c0 + dc * i;
            // The order of the state enumeration values is set up so that adding one
            // to the black or white state will put it in the start of the flip animation.
            board[r][c] = static_cast<TileState>(board[r][c] + 1);
        }
        return n;
    }

    void OthelloGame::update(int delta_ms) {
        State::update(delta_ms);

        if (buttons::b()) {
            ui::pop_state();
            return;
        }

        if (state == GS_ANIMATING_FLIPS) {
            animation_timer += delta_ms;
            if (animation_timer >= FLIP_ANIMATION_INTERVAL)
                animate_board();
        }

        else if (state == GS_WAITING_ON_PLAYER) {
            animation_timer += delta_ms;
            if (animation_timer >= CURSOR_ANIMATION_INTERVAL) {
                animation_counter = !animation_counter;
                animation_timer = 0;
            }

            if (buttons::left())
                cursor_col = (cursor_col + 7) % 8;
            else if (buttons::right())
                cursor_col = (cursor_col + 1) % 8;
            else if (buttons::up())
                cursor_row = (cursor_row + 7) % 8;
            else if (buttons::down())
                cursor_row = (cursor_row + 1) % 8;
            else if (buttons::a() || buttons::push())
                attempt_play();
        }
    }

    void OthelloGame::draw() {
        // Draw an empty board.
        drawing::clear(COLOR_BLACK);
        drawing::draw_rect(BOARD_LEFT - 1, BOARD_TOP - 1, BOARD_SIZE + 2, BOARD_SIZE + 2, BORDER_COLOR);
        drawing::fill_rect(BOARD_LEFT, BOARD_TOP, BOARD_SIZE, BOARD_SIZE, BG_COLOR);
        for (int i = 1; i < 8; i++) {
            const int x = BOARD_LEFT - 1 + i * TILE_SIZE;
            const int y = BOARD_TOP - 1 + i * TILE_SIZE;
            drawing::draw_line(x, BOARD_TOP, x, BOARD_TOP + BOARD_SIZE - 1, DIVIDER_COLOR);
            drawing::draw_line(BOARD_LEFT, y, BOARD_LEFT + BOARD_SIZE - 1, y, DIVIDER_COLOR);
        }

        // Draw the pieces and valid play markers on top of the board.
        const auto player_color = board.black_plays_next ? COLOR_BLACK : COLOR_WHITE;
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                const auto *image = get_tile_image(board.board[row][col]);
                if (image != nullptr)
                    drawing::draw_image(BOARD_LEFT + col * TILE_SIZE + PIECE_OFFSET,
                                        BOARD_TOP + row * TILE_SIZE + PIECE_OFFSET,
                                        *image);
                if (board.move_flips[row][col] > 0)
                    drawing::draw_ellipse(BOARD_LEFT + col * TILE_SIZE + PIECE_OFFSET,
                                          BOARD_TOP + row * TILE_SIZE + PIECE_OFFSET,
                                          TILE_SIZE - PIECE_OFFSET * 2 - 1,
                                          TILE_SIZE - PIECE_OFFSET * 2 - 1,
                                          player_color);
            }
        }

        // Draw the cursor.
        const auto &cursor_image = animation_counter == 0 ? image::cursor_anim1 : image::cursor_anim2;
        drawing::draw_image(BOARD_LEFT + cursor_row * TILE_SIZE + PIECE_OFFSET,
                            BOARD_TOP + cursor_col * TILE_SIZE + PIECE_OFFSET,
                            cursor_image);
    }

    void OthelloGame::pause() {}

    void OthelloGame::resume() { reset(); }

    void OthelloGame::reset() {
        state = GS_WAITING_ON_PLAYER;
        board = BoardState::initial_state();
        cursor_row = cursor_col = 2;
    }

    void OthelloGame::attempt_play() {
        if (board.make_play(cursor_row, cursor_col)) {
            state = GS_ANIMATING_FLIPS;
        }
        else {
            state = GS_ILLEGAL_MOVE;
        }
        animation_timer = animation_counter = 0;
    }

    void OthelloGame::animate_board() {
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                auto &tile = board.board[row][col];
                tile = get_next_state(tile);
            }
        }
        animation_timer = 0;
        animation_counter++;
        if (animation_counter >= 4) {
            state = GS_WAITING_ON_PLAYER;
            animation_counter = 0;
        }
    }

} // namespace othello
