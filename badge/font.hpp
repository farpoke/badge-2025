#pragma once

#include <string_view>

#include "font_data.hpp"

namespace font
{

    class Font {
    public:
        Font() = delete;
        explicit constexpr Font(const data::Font& data) : data(data) {}
        ~Font() = default;

        void draw(int x, int y, std::string_view text) const;

    private:
        const data::Font &data;
    };

    extern const Font lucida;
    extern const Font m5x7;
    extern const Font m6x11;
    extern const Font noto_sans;
    extern const Font noto_sans_cm;

}
