#include "snek.hpp"

#include <cassert>
#include <cstring>

#include <pico/rand.h>

#include <assets.hpp>
#include <badge/buttons.hpp>
#include <badge/drawing.hpp>
#include <badge/font.hpp>
#include <ui/ui.hpp>

namespace snek
{

    constexpr int CELL_SIZE   = 8;
    constexpr int GRID_WIDTH  = 19;
    constexpr int GRID_HEIGHT = 13;

    constexpr int GRID_PX_LEFT   = 4;
    constexpr int GRID_PX_TOP    = 20;
    constexpr int GRID_PX_WIDTH  = GRID_WIDTH * CELL_SIZE;
    constexpr int GRID_PX_HEIGHT = GRID_HEIGHT * CELL_SIZE;

    constexpr auto BG_COLOR     = COLOR_BLACK;
    constexpr auto BORDER_COLOR = COLOR_WHITE;
    constexpr auto SNEK_COLOR   = rgb888(50, 255, 0);

    constexpr auto INITIAL_SNEK_U      = GRID_WIDTH / 3;
    constexpr auto INITIAL_SNEK_V      = GRID_HEIGHT / 2;
    constexpr auto INITIAL_SNEK_LENGTH = 5;

    constexpr auto INITIAL_MOVE_INTERVAL   = 500;
    constexpr auto MOVE_INTERVAL_DECREMENT = 10;
    constexpr auto MIN_MOVE_INTERVAL       = 100;

    constexpr std::pair<int, int> direction_to_du_dv(CellState dir) {
        switch (dir) {
            case CellState::UP:
                return {0, -1};
            case CellState::DOWN:
                return {0, 1};
            case CellState::LEFT:
                return {-1, 0};
            case CellState::RIGHT:
                return {1, 0};
            default:
                panic("Invalid direction");
        }
    }

    void SnekGame::update(int delta_ms) {
        State::update(delta_ms);
        if (game_state == GameState::PLAYING) {
            move_timer -= delta_ms;
            if (move_timer <= 0) {
                advance();
                move_timer += move_interval;
                if (move_timer <= 0)
                    move_timer = move_interval;
                changed_direction = false;
                if (queued_direction != CellState::EMPTY) {
                    direction        = queued_direction;
                    queued_direction = CellState::EMPTY;
                }
            }
        }
        else if (game_state == GameState::DEAD) {
            move_timer -= delta_ms;
            if (move_timer <= 0)
                game_state = GameState::AFTERLIFE;
        }
        else if (game_state == GameState::AFTERLIFE) {
            if (buttons::a()) {
                reset();
            }
            else if (buttons::b()) {
                ui::pop_state();
            }
        }
        else if (game_state == GameState::WAITING_TO_START) {
            if (buttons::b())
                ui::pop_state();
        }
        if (buttons::up_current())
            steer(CellState::UP);
        if (buttons::down_current())
            steer(CellState::DOWN);
        if (buttons::left_current())
            steer(CellState::LEFT);
        if (buttons::right_current())
            steer(CellState::RIGHT);
    }

    void SnekGame::draw() {
        // First, clear to black.
        drawing::clear(COLOR_BLACK);

        if (game_state == GameState::WAITING_TO_START) {
            const auto render = font::m5x7.render("Press any direction to start");
            drawing::draw_text(lcd::WIDTH / 2 - render.dx - render.width / 2, GRID_PX_TOP + 2 - render.dy, 0, 0,
                               COLOR_WHITE, render);
            drawing::draw_image(lcd::WIDTH / 2 - image::nav_4way.width / 2, GRID_PX_TOP + 20, image::nav_4way);
        }
        else {
            // Write the score top and center.
            char       buffer[32];
            const auto n      = snprintf(buffer, sizeof(buffer), "Score: %d", score);
            const auto render = font::m6x11.render(std::string_view(buffer, n));
            drawing::draw_text(lcd::WIDTH / 2 - render.dx - render.width / 2, 2 - render.dy, 0, 0, COLOR_WHITE, render);
        }

        if (game_state == GameState::AFTERLIFE) {
            const auto press = font::m6x11.render("Press ");
            drawing::draw_text(10, 50, 0, 0, COLOR_WHITE, press);
            drawing::draw_image(10 + press.width, 50 + press.dy + (press.height - image::button_a.height) / 2,
                                image::button_a);
            drawing::draw_text(10, 90, 0, 0, COLOR_WHITE, press);
            drawing::draw_image(10 + press.width, 90 + press.dy + (press.height - image::button_b.height) / 2,
                                image::button_b);
            const auto to_restart = font::m6x11.render(" to restart");
            drawing::draw_text(10 + press.width + image::button_a.width, 50, 0, 0, COLOR_WHITE, to_restart);
            const auto to_exit = font::m6x11.render(" to exit");
            drawing::draw_text(10 + press.width + image::button_b.width, 90, 0, 0, COLOR_WHITE, to_exit);
            return;
        }

        // Draw the border around the play area.
        drawing::draw_rect(GRID_PX_LEFT - 2, GRID_PX_TOP - 2, GRID_PX_WIDTH + 4, GRID_PX_HEIGHT + 4, COLOR_WHITE);

        // Go over the grid and draw all the content.
        for (int u = 0; u < GRID_WIDTH; u++) {
            for (int v = 0; v < GRID_HEIGHT; v++) {
                const auto state = at(u, v);
                if (state == CellState::EMPTY)
                    continue;
                const auto left = GRID_PX_LEFT + u * CELL_SIZE;
                const auto top  = GRID_PX_TOP + v * CELL_SIZE;
                if (state == CellState::FRUIT)
                    drawing::draw_image(left, top, image::snek_fruit);
                else if (state == CellState::UP)
                    drawing::fill_rect(left + 1, top - 1, CELL_SIZE - 2, CELL_SIZE, SNEK_COLOR);
                else if (state == CellState::DOWN)
                    drawing::fill_rect(left + 1, top + 1, CELL_SIZE - 2, CELL_SIZE, SNEK_COLOR);
                else if (state == CellState::LEFT)
                    drawing::fill_rect(left - 1, top + 1, CELL_SIZE, CELL_SIZE - 2, SNEK_COLOR);
                else if (state == CellState::RIGHT)
                    drawing::fill_rect(left + 1, top + 1, CELL_SIZE, CELL_SIZE - 2, SNEK_COLOR);
                else if (state == CellState::HEAD) {
                    drawing::fill_rect(left + 1, top + 1, CELL_SIZE - 2, CELL_SIZE - 2, SNEK_COLOR);
                    int x, y, dx, dy;
                    if (direction == CellState::UP) {
                        x  = left + 2;
                        y  = top + 2;
                        dx = 1;
                        dy = 0;
                    }
                    else if (direction == CellState::DOWN) {
                        x  = left + CELL_SIZE - 3;
                        y  = top + CELL_SIZE - 3;
                        dx = -1;
                        dy = 0;
                    }
                    else if (direction == CellState::LEFT) {
                        x  = left + 2;
                        y  = top + CELL_SIZE - 3;
                        dx = 0;
                        dy = -1;
                    }
                    else if (direction == CellState::RIGHT) {
                        x  = left + CELL_SIZE - 3;
                        y  = top + 2;
                        dx = 0;
                        dy = 1;
                    }
                    else {
                        panic("Logic error in Snek");
                    }
                    // Draw eyes.
                    drawing::draw_pixel(x, y, COLOR_BLACK);
                    drawing::draw_pixel(x + dx * 3, y + dy * 3, COLOR_BLACK);
                    drawing::draw_pixel(x - dy, y + dx, COLOR_BLACK);
                    drawing::draw_pixel(x + dx * 3 - dy, y + dy * 3 + dx, COLOR_BLACK);
                    // Draw snout.
                    drawing::draw_line(x + dy * 2, y - dx * 2, x + dx * 3 + dy * 2, y + dy * 3 - dx * 2, SNEK_COLOR);
                }
            }
        }
    }

    void SnekGame::pause() {
        delete[] grid;
        grid = nullptr;
    }

    void SnekGame::resume() {
        assert(grid == nullptr);
        grid = new CellState[GRID_WIDTH * GRID_HEIGHT];

        reset();
    }

    CellState &SnekGame::at(int u, int v) {
        assert(u >= 0 && u < GRID_WIDTH && v >= 0 && v < GRID_HEIGHT);
        return grid[v * GRID_WIDTH + u];
    }

    void SnekGame::reset() {
        memset(grid, 0, sizeof(CellState) * GRID_WIDTH * GRID_HEIGHT);

        head_u    = INITIAL_SNEK_U;
        head_v    = INITIAL_SNEK_V;
        tail_u    = head_u - INITIAL_SNEK_LENGTH + 1;
        tail_v    = head_v;
        length    = INITIAL_SNEK_LENGTH;
        fruit_u   = GRID_WIDTH * 3 / 4;
        fruit_v   = GRID_HEIGHT * 3 / 4;
        direction = CellState::RIGHT;

        at(head_u, head_v) = CellState::HEAD;
        for (int i = 1; i < length; i++) {
            at(head_u - i, head_v) = CellState::RIGHT;
        }

        at(fruit_u, fruit_v) = CellState::FRUIT;

        move_timer    = INITIAL_MOVE_INTERVAL + 100;
        move_interval = INITIAL_MOVE_INTERVAL;

        changed_direction = false;
        queued_direction  = CellState::EMPTY;

        score = 0;

        game_state = GameState::WAITING_TO_START;
    }


    void SnekGame::advance() {
        auto &tail_state = at(tail_u, tail_v);

        const auto [head_du, head_dv] = direction_to_du_dv(direction);
        const auto [tail_du, tail_dv] = direction_to_du_dv(tail_state);

        const int new_head_u = (head_u + GRID_WIDTH + head_du) % GRID_WIDTH;
        const int new_head_v = (head_v + GRID_HEIGHT + head_dv) % GRID_HEIGHT;

        auto &new_head = at(new_head_u, new_head_v);

        bool ate_fruit = false;
        if (new_head == CellState::FRUIT) {
            new_head  = CellState::EMPTY;
            ate_fruit = true;
        }
        else {
            tail_state = CellState::EMPTY;
            tail_u     = (tail_u + GRID_WIDTH + tail_du) % GRID_WIDTH;
            tail_v     = (tail_v + GRID_HEIGHT + tail_dv) % GRID_HEIGHT;
        }

        if (new_head == CellState::EMPTY) {
            at(head_u, head_v) = direction;
            new_head           = CellState::HEAD;
            head_u             = new_head_u;
            head_v             = new_head_v;
        }
        else {
            game_state           = GameState::DEAD;
            at(fruit_u, fruit_v) = CellState::EMPTY;
            move_timer           = 1000;
        }

        if (ate_fruit) {
            length++;
            score += INITIAL_MOVE_INTERVAL + 100 - move_interval;
            if (move_interval > MIN_MOVE_INTERVAL)
                move_interval -= MOVE_INTERVAL_DECREMENT;
            place_fruit();
        }
    }

    void SnekGame::steer(CellState dir) {
        if (game_state == GameState::WAITING_TO_START)
            game_state = GameState::PLAYING;
        if (game_state != GameState::PLAYING)
            return;
        if (dir == direction)
            return;
        if ((direction == CellState::UP && dir == CellState::DOWN) ||
            (direction == CellState::DOWN && dir == CellState::UP) ||
            (direction == CellState::LEFT && dir == CellState::RIGHT) ||
            (direction == CellState::RIGHT && dir == CellState::LEFT))
            return;
        if (!changed_direction) {
            direction         = dir;
            changed_direction = true;
        }
        else {
            queued_direction = direction;
        }
    }

    void SnekGame::place_fruit() {
        int n = 0;
        for (int u = 0; u < GRID_WIDTH; u++) {
            for (int v = 0; v < GRID_HEIGHT; v++) {
                if (at(u, v) != CellState::EMPTY)
                    continue;
                n++;
                if (get_rand_32() % n == 0) {
                    fruit_u = u;
                    fruit_v = v;
                }
            }
        }
        at(fruit_u, fruit_v) = CellState::FRUIT;
    }


} // namespace snek
