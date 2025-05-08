#pragma once

#include <string>
#include <vector>

#include <badge/pixel.hpp>

#include "qr_code_galois.hpp"

namespace ui::qr
{

    enum class Version {
        V1_21x21 = 1,
        V2_25x25,
        V3_29x29,
        V4_33x33,
    };

    enum class ErrorCorrection {
        LOW = 0,
        MEDIUM,
        QUARTER,
        HIGH,
    };

    struct QrCode {
        Version version = {};
        ErrorCorrection ec = {};
        std::string content = {};

        void reset();
        void generate();
        void render(int scale);
        void draw(int left, int top) const;
        void print() const;

        int get_image_size() const { return image_size; }

    private:
        int size = {};
        std::vector<bool> data = {};
        std::vector<bool> reserved = {};
        int mask_index = {};

        int image_size = {};
        std::vector<Pixel> image = {};

        void set(int x, int y) { data[y * size + x] = true; }

        void add_finder(int x0, int y0);
        void add_alignment(int cx, int cy);
        void add_timing();
        void add_reserved_areas();
        void add_codewords(const std::vector<codeword_t>& data);
        void apply_mask();
        void add_info();

        std::vector<codeword_t> get_data_codewords() const;
        std::array<bool, 15> get_format_bits() const;
    };

} // namespace ui::qr
