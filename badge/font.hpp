#pragma once

#include <string_view>
#include <memory>

#include "font_data.hpp"

#include <badge/pixel.hpp>

namespace font
{

    struct TextMeasure {
        int left;
        int right;
        int top;
        int bottom;
        int advance;
    };

    struct TextDraw {
        TextDraw() = default;
        TextDraw(const TextDraw&) = delete;
        TextDraw(TextDraw&&) = default;
        ~TextDraw() = default;

        TextDraw& operator=(const TextDraw&) = delete;
        TextDraw& operator=(TextDraw&&) = default;

        int dx = 0;
        int dy = 0;
        int width = 0;
        int height = 0;
        std::unique_ptr<uint8_t[]> alpha = {};
    };

    class Font {
    public:
        Font() = delete;
        explicit constexpr Font(const data::Font& data) : data(data) {}
        ~Font() = default;

        TextMeasure measure(std::string_view text) const;
        TextDraw render(std::string_view text) const;

    private:
        const data::Font &data;
    };

    extern const Font lucida;
    extern const Font m5x7;
    extern const Font m6x11;
    extern const Font noto_sans;
    extern const Font noto_sans_cm;

}
