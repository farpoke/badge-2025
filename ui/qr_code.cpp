#include "qr_code.hpp"

#include <cassert>
#include <cstdio>

#include <pico.h>

#include <badge/drawing.hpp>

#include "qr_code_data.hpp"

namespace ui::qr
{

    codeword_t operator*(const codeword_t &lhs, const codeword_t &rhs) {
        if (lhs.value == 0 || rhs.value == 0)
            return {};
        const auto log = data::GALOIS_LOG_TABLE[lhs.value] + data::GALOIS_LOG_TABLE[rhs.value];
        const auto exp = data::GALOIS_EXP_TABLE[log % 255];
        return codeword_t(exp);
    }

    codeword_t operator/(const codeword_t &lhs, const codeword_t &rhs) {
        const auto log = data::GALOIS_LOG_TABLE[lhs.value] + data::GALOIS_LOG_TABLE[rhs.value] * 254;
        const auto exp = data::GALOIS_EXP_TABLE[log % 255];
        return codeword_t(exp);
    }

    polynomial_t operator*(const polynomial_t &lhs, const codeword_t &rhs) {
        if (lhs._coefficients == nullptr)
            return {};
        polynomial_t result = lhs;
        for (int i = 0; i <= result._degree; i++)
            result[i] = result[i] * rhs;
        return result;
    }

    polynomial_t operator*(const polynomial_t &lhs, const polynomial_t &rhs) {
        polynomial_t result;
        result._degree = lhs._degree + rhs._degree;
        result._coefficients = new codeword_t[result._degree + 1];
        for (int i_result = 0; i_result <= result._degree; i_result++) {
            codeword_t sum = {};
            for (int i_lhs = 0; i_lhs <= i_result; i_lhs++) {
                const int i_rhs = i_result - i_lhs;
                if (i_lhs <= lhs._degree && i_rhs >= 0 && i_rhs <= rhs._degree)
                    sum += lhs[i_lhs] * rhs[i_rhs];
            }
            result[i_result] = sum;
        }
        return result;
    }

    polynomial_t operator%(const polynomial_t &lhs, const polynomial_t &rhs) {
        polynomial_t temporary = lhs;
        int offset = 0;
        while (temporary._degree - offset >= rhs._degree) {
            const auto leading = temporary[offset];
            if (leading.value != 0) {
                const auto factor = leading / rhs[0];
                const auto subtract = rhs * factor;
                assert(subtract[0].value == leading.value);
                for (int i = 0; i <= subtract._degree; i++)
                    temporary[offset + i] -= subtract[i];
            }
            assert(temporary[offset].value == 0);
            offset++;
        }
        polynomial_t result;
        result._degree = temporary._degree - offset;
        result._coefficients = new codeword_t[result._degree + 1];
        for (int i = 0; i <= result._degree; i++)
            result[i] = temporary[i + offset];
        assert(result._degree == rhs._degree - 1);
        return result;
    }

    polynomial_t construct_generator_polynomial(int degree) {
        // Start with the degree 0 polynomial (1).
        polynomial_t result = {1};
        // Multiply degree 1 terms (x + 2**i) until we've reached our desired degree.
        polynomial_t term = {1, 0};
        for (int i = 0; i < degree; i++) {
            term[1].value = data::GALOIS_EXP_TABLE[i];
            result = result * term;
        }
        assert(result._degree == degree);
        return result;
    }

    std::vector<codeword_t> get_ec_codewords(const std::vector<codeword_t> &data, int degree) {
        const polynomial_t message_polynomial(data, degree);
        const auto generator = construct_generator_polynomial(degree);
        const auto remainder = message_polynomial % generator;
        const auto result = remainder.as_vector();
        assert(static_cast<int>(result.size()) == degree);
        return result;
    }

    void QrCode::reset() {
        size = 0;
        data = {};
        reserved = {};
        image_size = 0;
        image = {};
    }

    void QrCode::generate() {
        reset();

        size = 17 + 4 * static_cast<int>(version);
        data.resize(size * size);
        reserved.resize(size * size);
        for (int i = 0; i < size * size; i++)
            data[i] = reserved[i] = false;

        add_finder(0, 0);
        add_finder(size - 7, 0);
        add_finder(0, size - 7);
        if (version > Version::V1_21x21)
            add_alignment(size - 7, size - 7);
        add_timing();
        set(8, size - 8);
        add_reserved_areas();

        auto data_words = get_data_codewords();
        const auto n_ec_words = data::get_ec_codeword_count(version, ec);
        const auto ec_words = get_ec_codewords(data_words, n_ec_words);
        data_words.insert(data_words.end(), ec_words.begin(), ec_words.end());
        add_codewords(data_words);

        mask_index = 0;
        apply_mask();
        add_info();
    }

    void QrCode::render(int scale) {
        image_size = size * scale;
        image = {};
        image.resize(image_size * image_size);

        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                const auto color = data[y * size + x] ? COLOR_BLACK : COLOR_WHITE;
                for (int i = 0; i < scale; i++) {
                    for (int j = 0; j < scale; j++) {
                        const int px_x = x * scale + i;
                        const int px_y = y * scale + j;
                        image[px_x + px_y * image_size] = color;
                    }
                }
            }
        }
    }

    void QrCode::draw(int left, int top) const {
#ifndef TESTING
        drawing::copy(left, top, image_size, image_size, image_size, image.data());
#endif
    }

    void QrCode::print() const {
        const char* CHAR_0 = "█";
        const char* CHAR_1 = " ";
        for (int l = 0; l < 2; l++) {
            for (int i = 0; i < size + 4; i++)
                printf(CHAR_0);
            printf("\n");
        }
        for (int row = 0; row < size; row++) {
            printf(CHAR_0);
            printf(CHAR_0);
            for (int col = 0; col < size; col++) {
                if (data[row * size + col])
                    printf(CHAR_1);
                else
                    printf(CHAR_0);
            }
            printf(CHAR_0);
            printf(CHAR_0);
            printf("\n");
        }
        for (int l = 0; l < 2; l++) {
            for (int i = 0; i < size + 4; i++)
                printf(CHAR_0);
            printf("\n");
        }
    }

    void QrCode::add_finder(int x0, int y0) {
        for (int i = 0; i < 7; i++) {
            set(x0 + i, y0);
            set(x0 + i, y0 + 6);
            set(x0, y0 + i);
            set(x0 + 6, y0 + i);
        }
        for (int x = 2; x < 5; x++)
            for (int y = 2; y < 5; y++)
                set(x0 + x, y0 + y);
    }

    void QrCode::add_alignment(int cx, int cy) {
        set(cx, cy);
        for (int i = -2; i <= 2; i++) {
            set(cx - 2, cy + i);
            set(cx + 2, cy + i);
            set(cx + i, cy - 2);
            set(cx + i, cy + 2);
        }
    }

    void QrCode::add_timing() {
        for (int i = 8; i <= size - 8; i += 2) {
            set(6, i);
            set(i, 6);
        }
    }

    void QrCode::add_reserved_areas() {
        // Mark the top left finder pattern + info area as reserved.
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                reserved[i * size + j] = true;

        // Mark the other two finder patterns + info areas as reserved.
        for (int i = 0; i < 9; i++) {
            for (int j = size - 8; j < size; j++) {
                reserved[i * size + j] = true;
                reserved[j * size + i] = true;
            }
        }

        // Mark the alignment pattern as reserved.
        for (int i = size - 9; i <= size - 5; i++)
            for (int j = size - 9; j <= size - 5; j++)
                reserved[i * size + j] = true;

        // Mark timing patterns as reserved.
        for (int i = 8; i <= size - 8; i++) {
            reserved[i * size + 6] = true;
            reserved[6 * size + i] = true;
        }

        // Mark the dark module as reserved.
        reserved[(size - 8) * size + 8] = true;
    }

    void QrCode::add_codewords(const std::vector<codeword_t> &data) {

        int x = size - 1;
        int y = size - 1;
        bool going_up = true;
        bool going_left = true;

        auto place_bit = [&](bool bit) {
            assert(x >= 0 && x < size && y >= 0 && y <= size);
            assert(!reserved[y * size + x]);

            if (bit)
                set(x, y);

            do {
                if (going_left) {
                    x--;
                    going_left = false;
                }
                else {
                    x++;
                    going_left = true;
                    if (going_up) {
                        y--;
                        if (y < 0) {
                            y = 0;
                            x -= 2;
                            going_up = false;
                        }
                    }
                    else {
                        y++;
                        if (y >= size) {
                            y = size - 1;
                            x -= 2;
                            going_up = true;
                        }
                    }
                    if (x == 6) x--; // Skip past the vertical timing pattern.
                }
            } while (reserved[y * size + x]);
        };

        for (const auto& cw : data)
            for (int i = 7; i >= 0; i--)
                place_bit((cw.value >> i) & 1);
    }

    void QrCode::apply_mask() {
        auto inv_mask_func = [this](int row, int col) {
            switch (mask_index) {
            case 0: return (row + col) % 2;
            case 1: return row % 2;
            case 2: return col % 3;
            case 3: return (row + col) % 3;
            case 4: return (row / 2 + col / 3) % 2;
            case 5: return row * col % 2 + row * col % 3;
            case 6: return (row * col % 2 + row * col % 3) % 2;
            case 7: return ((row + col) % 2 + row * col % 3) % 2;
            default:
                panic("Invalid mask number");
            }
        };

        for (int row = 0; row < size; row++)
            for (int col = 0; col < size; col++)
                if (!reserved[row * size + col] && inv_mask_func(row, col) == 0)
                    data[row * size + col] = !data[row * size + col];
    }

    void QrCode::add_info() {
        const auto bits = get_format_bits();
        for (int i = 0; i < 7; i++) {
            if (!bits[i]) continue;
            set(i + (i > 5 ? 1 : 0), 8);
            set(8, size - 1 - i);
        }
        for (int i = 0; i < 8; i++) {
            if (!bits[7 + i]) continue;
            set(size - 8 + i, 8);
            set(8, 8 - i - (i > 1 ? 1 : 0));
        }
    }

    std::vector<codeword_t> QrCode::get_data_codewords() const {
        std::vector<codeword_t> result;
        const auto target_size = data::get_data_capacity(version, ec) + 2;
        result.reserve(target_size);

        // First byte contains 4 bits of encoding mode (0b0100 = byte encoding) and the upper nibble
        // of our content length. The second byte has the other nibble in its upper 4 bits.
        const auto len = static_cast<uint8_t>(content.size());
        result.emplace_back((0b0100 << 4) | (len >> 4));
        result.emplace_back(len << 4);

        // Now comes the data, which will always be split into upper and lower nibble in this mode.
        // The final byte will be left with four terminating zeroes in the lower nibble.
        for (const auto ch : content) {
            result.back().value |= ch >> 4;
            result.emplace_back(ch << 4);
        }

        // We now need to pad out any remaining space with alternating values of 236 and 17.
        while (static_cast<int>(result.size()) < target_size) {
            result.emplace_back(236);
            if (static_cast<int>(result.size()) < target_size)
                result.emplace_back(17);
        }

        // We now have a complete vector of bytes (code words) to process further.
        return result;
    }

    std::array<bool, 15> QrCode::get_format_bits() const {
        std::vector<codeword_t> coefficients;
        coefficients.resize(15);

        int ec_value;
        if (ec == ErrorCorrection::LOW)
            ec_value = 1;
        else if (ec == ErrorCorrection::MEDIUM)
            ec_value = 0;
        else if (ec == ErrorCorrection::QUARTER)
            ec_value = 3;
        else
            ec_value = 2;

        coefficients[0].value = ec_value >> 1;
        coefficients[1].value = ec_value & 1;
        coefficients[2].value = mask_index >> 2;
        coefficients[3].value = (mask_index >> 1) & 1;
        coefficients[4].value = mask_index & 1;

        const polynomial_t format_poly(coefficients, 0);
        const polynomial_t divider_poly {1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1};
        const auto remainder = format_poly % divider_poly;

        for (int i = 0; i < 9; i++)
            coefficients[5 + i] = remainder[i];

        constexpr std::array<int, 15> mask = {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0};
        for (int i = 0; i < 15; i++)
            coefficients[i].value ^= mask[i];

        std::array<bool, 15> result;
        for (int i = 0; i < 15; i++) {
            const auto value = coefficients[i].value;
            assert(value == 0 || value == 1);
            result[i] = value;
        }
        return result;
    }


} // namespace ui::qr
