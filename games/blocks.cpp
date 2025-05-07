#include "blocks.hpp"

#include <array>

#include <pico/rand.h>

#include <assets.hpp>
#include <badge/buttons.hpp>
#include <badge/drawing.hpp>
#include <badge/font.hpp>
#include <badge/pixel.hpp>
#include <ui/ui.hpp>

#include "blocks_data.hpp"

namespace blocks
{

    void BlocksGame::update(int delta_ms) {
        State::update(delta_ms);

        if (state == WAITING_TO_START) {
            if (buttons::a())
                state = PLAYING;
            else if (buttons::b())
                ui::pop_state();
        }
        else if (state == PLAYING) {
            fall_timer += delta_ms;
            if (fall_timer >= fall_interval) {
                fall();
            }

            if (buttons::left())
                shift_left();
            else if (buttons::right())
                shift_right();
            else if (buttons::up())
                hard_drop();
            else if (buttons::down_current()) {
                if (fall_timer >= SOFT_DROP_INTERVAL_MS)
                    fall();
            }
            else if (buttons::push() || buttons::a())
                rotate_cw();
            else if (buttons::b())
                hold();
        }
        else if (state == GAME_OVER) {
            if (buttons::a())
                reset();
            else if (buttons::b())
                ui::pop_state();
        }
    }

    void BlocksGame::draw() {
        drawing::clear(COLOR_BLACK);
        draw_field();
        draw_queue();
        draw_held();
    }

    void BlocksGame::pause() {}

    void BlocksGame::resume() { reset(); }

    void BlocksGame::reset() {
        // Completely clear the field.
        for (auto &row : field)
            for (auto &tile : row)
                tile = EMPTY;

        // Remove any current, held, or queueing pieces.
        current_piece = EMPTY;
        held_piece = EMPTY;
        piece_queue = {};

        // Reset overall game state.
        state = WAITING_TO_START;
        fall_interval = INITIAL_FALL_INTERVAL_MS;
        score = 0;

        // Spawn in the first piece. This will also reset current piece and queue.
        spawn_next();
    }

    void BlocksGame::spawn_next() {
        // Make sure we have enough pieces in the queue.
        while (piece_queue.size() <= NEXT_PIECE_COUNT)
            fill_queue();

        // Pick out the next piece from the queue.
        current_piece = piece_queue.front();
        piece_queue.pop_front();

        // Reset misc state for our new piece.
        current_piece_col = FIELD_WIDTH / 2 - 2;
        current_piece_row = FIELD_VISIBLE_HEIGHT;
        current_rotation = 0;
        hold_used = false;
        fall_timer = 0;

        update_ghost_row();

        // Check if the game is over.
        if (!try_place_piece(current_piece_row, current_piece_col, current_piece, current_rotation)) {
            state = GAME_OVER;
        }
    }

    void BlocksGame::fill_queue() {
        // Put one of each piece in a bag.
        std::array<EPiece, PIECE_COUNT> bag = {};
        for (int i = 0; i < PIECE_COUNT; i++)
            bag[i] = static_cast<EPiece>(i);

        // Shake up the bag.
        for (int reps = 0; reps < 2; reps++) {
            for (int i = 0; i < PIECE_COUNT; i++) {
                int j = get_rand_32() % PIECE_COUNT;
                if (i != j)
                    std::swap(bag[i], bag[j]);
            }
        }

        // Add the now random bag of pieces to the queue.
        for (auto piece : bag)
            piece_queue.push_back(piece);
    }

    void BlocksGame::fall() {
        if (try_place_piece(current_piece_row - 1, current_piece_col, current_piece, current_rotation)) {
            current_piece_row--;
            update_ghost_row();
        }
        else {
            hard_drop();
        }
        fall_timer = 0;
    }

    void BlocksGame::hard_drop() {
        update_ghost_row();

        for (int u = 0; u < 4; u++) {
            for (int v = 0; v < 4; v++) {
                if (!BLOCK_DATA[current_piece][current_rotation][u][v])
                    continue;
                const int r = ghost_row - v;
                const int c = current_piece_col + u;
                assert(r >= 0 && r < FIELD_HEIGHT && c >= 0 && c < FIELD_WIDTH);
                assert(field[r][c] == EMPTY);
                field[r][c] = current_piece;
            }
        }

        for (int r = 0; r < FIELD_HEIGHT; r++) {
            bool filled = true;
            for (int c = 0; c < FIELD_WIDTH; c++) {
                if (field[r][c] == EMPTY) {
                    filled = false;
                    break;
                }
            }
            if (!filled) continue;
            for (int r2 = r; r2 < FIELD_HEIGHT - 1; r2++) {
                for (int c = 0; c < FIELD_WIDTH; c++)
                    field[r2][c] = field[r2+1][c];
            }
            score += 100;
            r--;
        }

        spawn_next();
    }

    void BlocksGame::shift_left() {
        if (try_place_piece(current_piece_row, current_piece_col - 1, current_piece, current_rotation))
            current_piece_col--;
        update_ghost_row();
    }

    void BlocksGame::shift_right() {
        if (try_place_piece(current_piece_row, current_piece_col + 1, current_piece, current_rotation))
            current_piece_col++;
        update_ghost_row();
    }

    void BlocksGame::rotate_cw() {
        const int new_rotation = (current_rotation + 1) % 4;
        if (try_place_piece(current_piece_row, current_piece_col, current_piece, new_rotation))
            current_rotation = new_rotation;
        update_ghost_row();
    }

    void BlocksGame::rotate_ccw() {
        const int new_rotation = (current_rotation + 3) % 4;
        if (try_place_piece(current_piece_row, current_piece_col, current_piece, new_rotation))
            current_rotation = new_rotation;
        update_ghost_row();
    }

    void BlocksGame::hold() {
        if (hold_used)
            return;
        if (held_piece != EMPTY)
            piece_queue.push_front(held_piece);
        held_piece = current_piece;
        spawn_next();
        hold_used = true;
    }

    bool BlocksGame::try_place_piece(int row, int col, EPiece piece, int rotation) const {
        for (int u = 0; u < 4; u++) {
            for (int v = 0; v < 4; v++) {
                if (!BLOCK_DATA[piece][rotation][u][v])
                    continue;
                const int r = row - v;
                const int c = col + u;
                if (r < 0 || r >= FIELD_HEIGHT || c < 0 || c >= FIELD_WIDTH)
                    return false;
                if (field[r][c] != EMPTY)
                    return false;
            }
        }
        return true;
    }

    void BlocksGame::update_ghost_row() {
        ghost_row = current_piece_row;
        while (try_place_piece(ghost_row - 1, current_piece_col, current_piece, current_rotation))
            ghost_row--;
    }

    void BlocksGame::draw_field() const {

        // Draw the border around the playing field.
        drawing::draw_rect(FIELD_PX_LEFT - 1,
                           FIELD_PX_BOTTOM + 1,
                           FIELD_WIDTH * TILE_SIZE + 2,
                           -FIELD_VISIBLE_HEIGHT * TILE_SIZE,
                           COLOR_WHITE);

        // Draw the field tiles.
        for (int c = 0; c < FIELD_WIDTH; c++) {
            for (int r = 0; r < FIELD_VISIBLE_HEIGHT; r++) {
                if (field[r][c] == EMPTY)
                    continue;
                const int dst_left = FIELD_PX_LEFT + c * TILE_SIZE;
                const int dst_top = FIELD_PX_BOTTOM - (r + 1) * TILE_SIZE;
                const int src_left = TILE_SIZE * (field[r][c] + 1);
                drawing::draw_image(dst_left, dst_top, src_left, 0, TILE_SIZE, TILE_SIZE, image::blocks_tiles);
            }
        }

        // Draw the ghost piece.
        if (state == PLAYING) {
            draw_piece(FIELD_PX_LEFT + TILE_SIZE * current_piece_col,
                       FIELD_PX_BOTTOM - TILE_SIZE * (ghost_row + 1),
                       current_piece,
                       current_rotation,
                       true);
        }

        // Draw the current piece. Possibly on top of the ghost piece.
        draw_piece(FIELD_PX_LEFT + TILE_SIZE * current_piece_col,
                   FIELD_PX_BOTTOM - TILE_SIZE * (current_piece_row + 1),
                   current_piece,
                   current_rotation,
                   false);
    }

    void BlocksGame::draw_queue() const {

        // Draw the border around the next piece queue.
        drawing::draw_rect(QUEUE_PX_LEFT - 2,
                           QUEUE_PX_TOP - 2,
                           TILE_SIZE * 4 + 4,
                           TILE_SIZE * 2 * NEXT_PIECE_COUNT + QUEUE_PX_SPACING * (NEXT_PIECE_COUNT - 1) + 4,
                           COLOR_WHITE);

        // Draw the text label above the queue.
        drawing::draw_text_centered(QUEUE_PX_LEFT + TILE_SIZE * 2, QUEUE_PX_TOP - 5, "Next:", COLOR_WHITE, font::m6x11);

        // Draw the queue pieces.
        for (int i = 0; i < NEXT_PIECE_COUNT; i++) {
            draw_piece(QUEUE_PX_LEFT,
                       QUEUE_PX_TOP + (TILE_SIZE * 2 + QUEUE_PX_SPACING) * i,
                       piece_queue.at(i),
                       0,
                       false);
        }
    }

    void BlocksGame::draw_held() const {

        // Draw the border around the held piece area.
        drawing::draw_rect(HOLD_PX_LEFT - 2, HOLD_PX_TOP - 2, TILE_SIZE * 4 + 4, TILE_SIZE * 2 + 4, COLOR_WHITE);

        // Draw the text label above the holding area.
        drawing::draw_text_centered(HOLD_PX_LEFT + TILE_SIZE * 2, HOLD_PX_TOP - 5, "Held:", COLOR_WHITE, font::m6x11);

        // Draw the held piece.
        if (held_piece != EMPTY) {
            draw_piece(HOLD_PX_LEFT, HOLD_PX_TOP, held_piece, 0, false);
        }
    }

    void BlocksGame::draw_piece(int left, int top, EPiece piece, int rotation, bool is_ghost) {
        const auto src_left = is_ghost ? 0 : TILE_SIZE * (piece + 1);
        for (int u = 0; u < 4; u++) {
            for (int v = 0; v < 4; v++) {
                if (!BLOCK_DATA[piece][rotation][u][v])
                    continue;
                const int x = left + u * TILE_SIZE;
                const int y = top + v * TILE_SIZE;
                drawing::draw_image(x, y, src_left, 0, TILE_SIZE, TILE_SIZE, image::blocks_tiles);
            }
        }
    }

} // namespace blocks
