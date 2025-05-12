#pragma once

#include <cstdint>
#include <string>

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
        MISC_SOCIAL,
        MISC_LITERAL1,
        MISC_LITERAL2,

        ARDUINO_MORSE,
        ARDUINO_SERIAL,

        CRYPTO_CAESAR,

        LOCKPICK_BASIC,
        LOCKPICK_ELITE,

        FLAG_COUNT,

        INVALID,
    };

    Flag validate_flag(const std::string& flag);
    bool has_flag(Flag flag);
    int count_flags();

    std::string get_konami_code();

}
