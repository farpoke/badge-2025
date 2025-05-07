#include "othello.hpp"

#include <assets.hpp>
#include <badge/drawing.hpp>
#include <badge/pixel.hpp>

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
        switch (tile) {
        case TS_WHITE: return TS_FLIP_W2B_1;
        case TS_FLIP_W2B_1: return TS_FLIP_W2B_2;
        case TS_FLIP_W2B_2: return TS_FLIP_W2B_3;
        case TS_FLIP_W2B_3: return TS_BLACK;
        case TS_BLACK: return TS_FLIP_B2W_1;
        case TS_FLIP_B2W_1: return TS_FLIP_B2W_2;
        case TS_FLIP_B2W_2: return TS_FLIP_B2W_3;
        case TS_FLIP_B2W_3: return TS_WHITE;
        default: return tile;
        }
    }

    void OthelloGame::update(int delta_ms) {
        State::update(delta_ms);

        animation_timer += delta_ms;
        if (animation_timer >= FLIP_ANIMATION_INTERVAL) {
            animate_board();
            animation_timer = 0;
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

        // Draw the pieces on top of the board.
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                const auto *image = get_tile_image(board[row][col]);
                if (image != nullptr)
                    drawing::draw_image(BOARD_LEFT + col * TILE_SIZE + PIECE_OFFSET,
                                        BOARD_TOP + row * TILE_SIZE + PIECE_OFFSET,
                                        *image);
            }
        }
    }

    void OthelloGame::pause() {}

    void OthelloGame::resume() { reset(); }

    void OthelloGame::reset() {
        // Clear the board.
        for (auto &row : board)
            for (auto &tile : row)
                tile = TS_EMPTY;

        // Add in the fixed starting pieces.
        board[3][3] = board[4][4] = TS_WHITE;
        board[3][4] = board[4][3] = TS_BLACK;

        // Reset state machine.
        state = GS_WAITING_ON_PLAYER;
        white_plays_next = false;
    }

    void OthelloGame::animate_board() {
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                auto &tile = board[row][col];
                tile = get_next_state(tile);
            }
        }
    }

} // namespace othello
