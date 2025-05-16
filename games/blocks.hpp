#pragma once

#include <array>
#include <deque>

#include <badge/pixel.hpp>
#include <ui/state.hpp>

/**
 * Legally distinct implementation of Tetris.
 * Attempts to follow the Tetris Guidelines.
 *
 * See https://tetris.fandom.com/wiki/Tetris_Guideline
 */
namespace blocks
{

    /// Tile size in pixels. 6 is the largest size that still fits a full field on the LCD.
    constexpr auto TILE_SIZE = 6;

    /// Field width in tiles. 10 is the value from the guidelines.
    constexpr auto FIELD_WIDTH = 10;

    /// Field height in tiles. 40 is the value from the guidelines, for some reason.
    constexpr auto FIELD_HEIGHT = 40;

    /// Number of visible rows of tiles of the field. 20 is the value from the guidelines, with a
    /// few additional rows of pixels visible for the 21st row.
    constexpr auto FIELD_VISIBLE_HEIGHT = 21;

    /// How many "next pieces" to show next to the field.
    /// The guidelines request 1 to 6, and we have just enough space to comfortably show six.
    constexpr auto NEXT_PIECE_COUNT = 6;

    /// Initial number of milliseconds between "ticks" where the current piece falls one tile.
    /// The actual value during play will go lower as the player levels up.
    constexpr auto INITIAL_FALL_INTERVAL_MS = 800;

    /// Milliseconds between ticks where the current piece falls one tile when the player holds (down).
    constexpr auto SOFT_DROP_INTERVAL_MS = 200;

    /// Leftmost column of pixels of the playing field.
    /// Calculated to center the field on the LCD.
    constexpr auto FIELD_PX_LEFT = (160 - TILE_SIZE * FIELD_WIDTH) / 2;

    /// Bottommost row of pixels of the playing field.
    /// Calculated so that only half of the upper row of tiles are visible.
    constexpr auto FIELD_PX_BOTTOM = TILE_SIZE * (FIELD_VISIBLE_HEIGHT - 1) + TILE_SIZE / 2;

    constexpr auto QUEUE_PX_LEFT    = 13;        ///< Position of the left edge of the "next piece" queue display.
    constexpr auto QUEUE_PX_TOP     = 20;        ///< Position of the top edge of the "next piece" queue display.
    constexpr auto QUEUE_PX_SPACING = TILE_SIZE; ///< Spacing between pieces in the "next piece" queue display.

    constexpr auto HOLD_PX_LEFT = 123;          ///< Position of the left edge of the held piece display.
    constexpr auto HOLD_PX_TOP  = QUEUE_PX_TOP; ///< Position of the top edge of the held piece display.

    enum EPiece {
        PIECE_O = 0,
        PIECE_I,
        PIECE_L,
        PIECE_J,
        PIECE_S,
        PIECE_Z,
        PIECE_T,
        PIECE_COUNT,

        EMPTY = -1
    };

    static_assert(PIECE_COUNT == 7);

    class BlocksGame final : public ui::State {
    public:
        void update(int delta_ms) override;
        void draw() override;

        void pause() override;
        void resume() override;

    protected:

        enum GameState {
            WAITING_TO_START,
            PLAYING,
            GAME_OVER,
        };

        GameState state = WAITING_TO_START;

        /// Playing field, addressed as field[row-from-bottom][column-from-left]
        std::array<std::array<EPiece, FIELD_WIDTH>, FIELD_HEIGHT> field = {};

        EPiece current_piece     = EMPTY; ///< Current falling piece.
        int    current_piece_row = 0;     ///< Current piece row position.
        int    current_piece_col = 0;     ///< Current piece column position.
        int    current_rotation  = 0;     ///< Current piece rotation (0-3).

        int ghost_row = 0; ///< Row position of the current ghost piece. Column is the same as the current piece.

        EPiece held_piece = EMPTY; ///< Currently held piece, or `EMPTY`.
        bool   hold_used  = false; ///< True iff the hold function has been used this piece.

        /// Queue of pieces to appear next.
        std::deque<EPiece> piece_queue = {};

        int fall_timer    = 0; ///< Timer counting up in milliseconds from when the current piece fell one tile.
        int fall_interval = 0; ///< Current interval between falls.

        int soft_drop_count = 0; ///< Count how many spaces the current piece has been soft-dropped, for scoring.

        bool last_move_was_spin = false; ///< Keep track of if the last successful move was a spin, for T-spin scoring.

        /// Current game score.
        int score = 0;

        /// Current level.
        int level = 0;

        /// Reset everything to a new initial state.
        void reset();

        /// Spawn the next piece to become the current piece.
        void spawn_next();

        /// Randomize pieces and add to the end of the queue.
        void fill_queue();

        /// Make the current piece fall one tile.
        void fall();

        /// Hard-drop the current piece. This function is responsible for doing the final
        /// piece placement logic, and subsequent clearing of rows, updating score etc.
        void hard_drop();

        /// Try to move the current piece one tile left.
        void shift_left();

        /// Try to move the current piece one tile right.
        void shift_right();

        /// Try to rotate the current piece clockwise.
        void rotate_cw();

        /// Try to rotate the current piece counter-clockwise.
        void rotate_ccw();

        /// Swap out the current piece for the held piece.
        void hold();

        /// Try to place a given piece at the given location.
        /// Return true iff the piece fits.
        /// This is a const function, i.e. no actual state is modified.
        bool try_place_piece(int row, int col, EPiece piece, int rotation) const;

        /// Figure out where the current piece will end up if hard-dropped, so we can display the ghost
        /// piece and/or actually do the hard-drop.
        void update_ghost_row();

        /// Draw the playing field.
        void draw_field() const;

        /// Draw the "next piece" queue display.
        void draw_queue() const;

        /// Draw the "held piece" display.
        void draw_held() const;

        /// Draw a piece somewhere on the screen.
        static void draw_piece(int left, int top, EPiece piece, int rotation, bool is_ghost);

    };

} // namespace blocks
