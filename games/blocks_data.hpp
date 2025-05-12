#pragma once

namespace blocks
{

    // clang-format off
    /**
     * Descriptions of the various block shapes in constant string form.
     * These are translated to more computationally convenient form by
     * `consteval` functions, instead of being used directly.
     *
     * Each piece has four rotations (though for the O piece they are
     * identical). Each rotation is described by 4x4 characters,
     * either `.` for empty, or `#` for occupied space.
     *
     * The first rotation is always the initial one for when the piece
     * first spawns in. The centers of rotation are nominally at the
     * center of tile (1, 1), but the O and I pieces are special cases.
     *
     * See https://tetris.fandom.com/wiki/SRS
     */
    constexpr const char* BLOCK_DESCRIPTIONS[PIECE_COUNT][4] = {
        {
            // O piece
            ".##."
            ".##."
            "...."
            "....",

            ".##."
            ".##."
            "...."
            "....",

            ".##."
            ".##."
            "...."
            "....",

            ".##."
            ".##."
            "...."
            "...."
        },
        {
            // I piece
            "...."
            "####"
            "...."
            "....",

            "..#."
            "..#."
            "..#."
            "..#.",

            "...."
            "...."
            "####"
            "....",

            ".#.."
            ".#.."
            ".#.."
            ".#.."
        },
        {
            // L piece
            "..#."
            "###."
            "...."
            "....",

            ".#.."
            ".#.."
            ".##."
            "....",

            "...."
            "###."
            "#..."
            "....",

            "##.."
            ".#.."
            ".#.."
            "...."
        },
        {
            // J piece
            "#..."
            "###."
            "...."
            "....",

            ".##."
            ".#.."
            ".#.."
            "....",

            "...."
            "###."
            "..#."
            "....",

            ".#.."
            ".#.."
            "##.."
            "...."
        },
        {
            // S piece
            ".##."
            "##.."
            "...."
            "....",

            ".#.."
            ".##."
            "..#."
            "....",

            "...."
            ".##."
            "##.."
            "....",

            "#..."
            "##.."
            ".#.."
            "...."
        },
        {
            // Z piece
            "##.."
            ".##."
            "...."
            "....",

            "..#."
            ".##."
            ".#.."
            "....",

            "...."
            "##.."
            ".##."
            "....",

            ".#.."
            "##.."
            "#..."
            "...."
        },
        {
            // T piece
            ".#.."
            "###."
            "...."
            "....",

            ".#.."
            ".##."
            ".#.."
            "....",

            "...."
            "###."
            ".#.."
            "....",

            ".#.."
            "##.."
            ".#.."
            "...."
        }
    };
    // clang-format on

    using block_rotation_t = std::array<std::array<bool, 4>, 4>;
    using block_data_t = std::array<block_rotation_t, 4>;
    using blocks_t = std::array<block_data_t, PIECE_COUNT>;

    consteval block_rotation_t description_to_rotation(auto text) {
        block_rotation_t result = {};
        for (int u = 0; u < 4; u++)
            for (int v = 0; v < 4; v++)
                result[u][v] = text[v * 4 + u] == '#';
        return result;
    }

    consteval block_data_t descriptions_to_rotations(auto descriptions) {
        block_data_t result = {};
        for (int r = 0, i = 0; r < 4; r++, i++) {
            if (descriptions[i] == nullptr)
                i = 0;
            result[r] = description_to_rotation(descriptions[i]);
        }
        return result;
    }

    consteval blocks_t compute_block_data() {
        blocks_t result = {};
        for (int i = 0; i < PIECE_COUNT; i++)
            result[i] = descriptions_to_rotations(BLOCK_DESCRIPTIONS[i]);
        return result;
    }

    constexpr blocks_t BLOCK_DATA = compute_block_data();

    using kick_offset_t = std::pair<int, int>;
    using kick_tests_t = std::array<kick_offset_t, 4>;

    struct kick_data_t {
        std::array<kick_tests_t, 4> cw_kicks;
        std::array<kick_tests_t, 4> ccw_kicks;
    };

    /**
     * Wall/super kick data table for the I piece.
     *
     * See https://tetris.fandom.com/wiki/SRS
     */
    constexpr kick_data_t I_KICK_DATA = {
            {
                    // CW kicks

                    // Rotation 0 -> 1
                    kick_tests_t{{
                            {-2, 0},
                            {1, 0},
                            {-2, 1},
                            {1, 2},
                    }},

                    // Rotation 1 -> 2
                    kick_tests_t{{
                            {-1, 0},
                            {2, 0},
                            {-1, 2},
                            {2, -1},
                    }},

                    // Rotation 2 -> 3
                    kick_tests_t{{
                            {2, 0},
                            {-1, 0},
                            {2, 1},
                            {-1, -2},
                    }},

                    // Rotation 3 -> 0
                    kick_tests_t{{
                            {1, 0},
                            {-2, 0},
                            {1, -2},
                            {-2, 1},
                    }},
            },
            {
                    // CCW kicks

                    // Rotation 0 -> 3
                    kick_tests_t{{
                            {-1, 0},
                            {2, 0},
                            {-1, 2},
                            {2, -1},
                    }},

                    // Rotation 1 -> 0
                    kick_tests_t{{
                            {2, 0},
                            {-1, 0},
                            {2, 1},
                            {-1, -2},
                    }},

                    // Rotation 2 -> 1
                    kick_tests_t{{
                            {1, 0},
                            {-2, 0},
                            {1, -2},
                            {-2, 1},
                    }},

                    // Rotation 3 -> 2
                    kick_tests_t{{
                            {-2, 0},
                            {1, 0},
                            {-2, -1},
                            {1, 2},
                    }},
            },
    };

    /**
     * Wall/super kick data table for the J, L, T, S, and Z pieces.
     *
     * See https://tetris.fandom.com/wiki/SRS
     */
    constexpr kick_data_t JLTSZ_KICK_DATA = {
            {
                    // CW kicks

                    // Rotation 0 -> 1
                    kick_tests_t{{
                            {-1, 0},
                            {-1, 1},
                            {0, -2},
                            {-1, -2},
                    }},

                    // Rotation 1 -> 2
                    kick_tests_t{{
                            {1, 0},
                            {1, -1},
                            {0, 2},
                            {1, 2},
                    }},

                    // Rotation 2 -> 3
                    kick_tests_t{{
                            {1, 0},
                            {1, 1},
                            {0, -2},
                            {1, -2},
                    }},

                    // Rotation 3 -> 0
                    kick_tests_t{{
                            {-1, 0},
                            {-1, -1},
                            {0, 2},
                            {-1, 2},
                    }},
            },
            {
                    // CCW kicks

                    // Rotation 0 -> 3
                    kick_tests_t{{
                            {1, 0},
                            {1, 1},
                            {0, -2},
                            {1, -2},
                    }},

                    // Rotation 1 -> 0
                    kick_tests_t{{
                            {1, 0},
                            {1, -1},
                            {0, 2},
                            {1, 2},
                    }},

                    // Rotation 2 -> 1
                    kick_tests_t{{
                            {-1, 0},
                            {-1, 1},
                            {0, -2},
                            {-1, -2},
                    }},

                    // Rotation 3 -> 2
                    kick_tests_t{{
                            {-1, 0},
                            {-1, -1},
                            {0, 2},
                            {-1, 2},
                    }},
            },
    };

} // namespace blocks
