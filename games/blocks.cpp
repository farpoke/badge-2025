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
            else if (buttons::up() || buttons::d())
                hard_drop();
            else if (buttons::down_current()) {
                if (fall_timer >= SOFT_DROP_INTERVAL_MS)
                    fall();
            }
            else if (buttons::push() || buttons::a())
                rotate_cw();
            else if (buttons::b())
                hold();
            else if (buttons::c())
                rotate_ccw();

            if (!buttons::down_current())
                soft_drop_count = 0;
        }
        else if (state == GAME_OVER) {
            if (buttons::a()) {
                reset();
                state = PLAYING;
            }
            else if (buttons::b())
                ui::pop_state();
        }
    }

    void BlocksGame::draw() {
        drawing::clear(COLOR_BLACK);

        draw_field();
        draw_queue();
        draw_held();

        if (state == WAITING_TO_START || state == GAME_OVER) {
            drawing::fill_rect(10, 50, 140, 50, COLOR_BLACK, 220);
            drawing::draw_rect(10, 50, 140, 50, COLOR_WHITE);
            const auto press = font::m6x11.render("Press ");
            drawing::draw_text(20, 70, 0, 0, COLOR_WHITE, press);
            drawing::draw_image(20 + press.width,
                                70 + press.dy + (press.height - image::button_a.height) / 2,
                                image::button_a);
            drawing::draw_text(20, 90, 0, 0, COLOR_WHITE, press);
            drawing::draw_image(20 + press.width,
                                90 + press.dy + (press.height - image::button_b.height) / 2,
                                image::button_b);
            const auto to_restart = font::m6x11.render(" to start");
            drawing::draw_text(20 + press.width + image::button_a.width, 70, 0, 0, COLOR_WHITE, to_restart);
            const auto to_exit = font::m6x11.render(" to exit");
            drawing::draw_text(20 + press.width + image::button_b.width, 90, 0, 0, COLOR_WHITE, to_exit);
        }

        if (state != WAITING_TO_START) {
            drawing::draw_text(FIELD_PX_LEFT + FIELD_WIDTH * TILE_SIZE + 5, FIELD_PX_BOTTOM - 20, "Score:", COLOR_WHITE, font::m6x11);
            char buffer[32];
            int n = snprintf(buffer, sizeof(buffer), "%d", score);
            std::string text(buffer, n);
            drawing::draw_text(FIELD_PX_LEFT + FIELD_WIDTH * TILE_SIZE + 5, FIELD_PX_BOTTOM - 5, text, COLOR_WHITE, font::m6x11);
        }
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
        level = 1;

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
        last_move_was_spin = false;

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
            last_move_was_spin = false;
            update_ghost_row();
        }
        else {
            hard_drop();
        }
        fall_timer = 0;
        soft_drop_count++;
    }

    void BlocksGame::hard_drop() {
        update_ghost_row();

        int bonus_score = 0;

        // Add hard drop bonus score.
        if (current_piece_row > ghost_row)
            bonus_score += 2 * (current_piece_row - ghost_row);

        // Add soft drop bonus score.
        bonus_score += soft_drop_count;

        // Place piece onto the field.
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

        // If we dropped a T piece, figure out if this was a valid T-spin.
        bool t_spin = false;
        if (current_piece == PIECE_T && last_move_was_spin) {
            int t_spin_count = 0;
            std::array coords = {
                    std::pair{ghost_row, current_piece_col},
                    std::pair{ghost_row, current_piece_col + 2},
                    std::pair{ghost_row + 2, current_piece_col},
                    std::pair{ghost_row + 2, current_piece_col + 2},
            };
            for (const auto &[r, c] : coords) {
                if (r < 0 || r >= FIELD_HEIGHT || c < 0 || c >= FIELD_WIDTH || field[r][c] != EMPTY)
                    t_spin_count++;
            }
            if (t_spin_count >= 3)
                t_spin = true;
        }

        // Count and remove cleared rows.
        int n_cleared = 0;
        for (int r = 0; r < FIELD_HEIGHT; r++) {
            bool filled = true;
            for (int c = 0; c < FIELD_WIDTH; c++) {
                if (field[r][c] == EMPTY) {
                    filled = false;
                    break;
                }
            }
            if (!filled)
                continue;
            for (int r2 = r; r2 < FIELD_HEIGHT - 1; r2++) {
                for (int c = 0; c < FIELD_WIDTH; c++)
                    field[r2][c] = field[r2 + 1][c];
            }
            n_cleared++;
            r--;
        }

        // Add score.
        if (n_cleared > 0)
            score += bonus_score;
        if (t_spin) {
            if (n_cleared == 0)
                score += 100 * level;
            else if (n_cleared == 1)
                score += 200 * level;
            else if (n_cleared == 2)
                score += 400 * level;
            else if (n_cleared == 3)
                score += 800 * level;
        }
        else {
            if (n_cleared == 1)
                score += 100 * level;
            else if (n_cleared == 2)
                score += 300 * level;
            else if (n_cleared == 3)
                score += 500 * level;
            else if (n_cleared == 4)
                score += 800 * level;
        }

        // Spawn the next piece.
        spawn_next();
    }

    void BlocksGame::shift_left() {
        if (try_place_piece(current_piece_row, current_piece_col - 1, current_piece, current_rotation)) {
            current_piece_col--;
            last_move_was_spin = false;
            update_ghost_row();
        }
    }

    void BlocksGame::shift_right() {
        if (try_place_piece(current_piece_row, current_piece_col + 1, current_piece, current_rotation)) {
            current_piece_col++;
            last_move_was_spin = false;
            update_ghost_row();
        }
    }

    void BlocksGame::rotate_cw() {
        if (current_piece == PIECE_O)
            return;
        const int new_rotation = (current_rotation + 1) % 4;
        if (try_place_piece(current_piece_row, current_piece_col, current_piece, new_rotation)) {
            current_rotation = new_rotation;
            last_move_was_spin = true;
        }
        else {
            const auto &[cw_kicks, _] = (current_piece == PIECE_I) ? I_KICK_DATA : JLTSZ_KICK_DATA;
            for (const auto &[dx, dy] : cw_kicks[current_rotation]) {
                if (try_place_piece(current_piece_row + dy, current_piece_col + dx, current_piece, new_rotation)) {
                    current_rotation = new_rotation;
                    current_piece_col += dx;
                    current_piece_row += dy;
                    last_move_was_spin = true;
                    break;
                }
            }
        }
        update_ghost_row();
    }

    void BlocksGame::rotate_ccw() {
        if (current_piece == PIECE_O)
            return;
        const int new_rotation = (current_rotation + 3) % 4;
        if (try_place_piece(current_piece_row, current_piece_col, current_piece, new_rotation)) {
            current_rotation = new_rotation;
            last_move_was_spin = true;
        }
        else {
            const auto &[_, ccw_kicks] = (current_piece == PIECE_I) ? I_KICK_DATA : JLTSZ_KICK_DATA;
            for (const auto &[dx, dy] : ccw_kicks[current_rotation]) {
                if (try_place_piece(current_piece_row + dy, current_piece_col + dx, current_piece, new_rotation)) {
                    current_rotation = new_rotation;
                    current_piece_col += dx;
                    current_piece_row += dy;
                    last_move_was_spin = true;
                    break;
                }
            }
        }
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
