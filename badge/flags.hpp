#pragma once

#include <cstdint>
#include <string>
#include <vector>

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
        MISC_MVP,

        ARDUINO_MORSE,
        ARDUINO_SERIAL,

        CRYPTO_CAESAR,

        LOCKPICK_BASIC,
        LOCKPICK_ELITE,
        LOCKPICK_DIY,

        WEB_EASY,
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

        EXPLORER_1,
        EXPLORER_2,
        EXPLORER_3,

        FLAG_COUNT,

        INVALID,
    };

    void init();
    Flag enter_flag(const std::string& text);
    const std::vector<Flag>& get_found_flags();

    std::string get_konami_code();

    const image::Image& get_flag_image(Flag flag);

}
