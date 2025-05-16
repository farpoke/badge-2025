#pragma once

#include <cstdint>
#include <string>

#include <badge/image.hpp>

namespace flags
{

    enum Flag : uint8_t {
        BADGE_README = 0,
        BADGE_HIDDEN,
        BADGE_KONAMI,
        BADGE_RICKROLL,
        BADGE_PI,
        BADGE_BAUDOT,

        MISC_REBEKAH,
        MISC_LITERAL1,
        MISC_LITERAL2,

        ARDUINO_MORSE,
        ARDUINO_SERIAL,

        CRYPTO_CAESAR,

        LOCKPICK_BASIC,
        LOCKPICK_ELITE,

        WEB_MEDIUM,

        PWN_MEDIUM,
        PWN_ELITE,

        RE_EASY,
        RE_MEDIUM,
        RE_ELITE,

        STEGO_EASY,
        STEGO_ELITE,

        BASIC_2024_HASH,
        BASIC_2024_LOCK,
        BASIC_2024_CRYPTO,
        BASIC_2024_CRED,

        ELITE_2024_HASH,
        ELITE_2024_SOCIAL,
        ELITE_2024_CRED,

        FLAG_COUNT,

        INVALID,
    };

    void init();
    Flag enter_flag(const std::string& text);
    bool has_flag(Flag flag);
    int count_flags();

    std::string get_konami_code();

    const image::Image& get_flag_image(Flag flag);

}
