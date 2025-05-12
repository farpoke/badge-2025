#include "flags.hpp"

#include <array>
#include <cassert>
#include <cstdint>

#include <assets.hpp>

namespace flags
{

#include "flags-data.inc"

    consteval std::array<uint8_t, MAX_FLAG_LENGTH * FLAG_COUNT> obfuscate_flags() {
        std::array<uint8_t, MAX_FLAG_LENGTH * FLAG_COUNT> result = {};
        for (int i = 0; i < FLAG_COUNT; i++) {
            const auto flag = get_plaintext_flag(static_cast<Flag>(i));
            assert(flag.length() <= MAX_FLAG_LENGTH);
            for (size_t j = 0; j < MAX_FLAG_LENGTH; j++) {
                if (j < flag.length())
                    result[i * MAX_FLAG_LENGTH + j] = flag[j];
                else
                    result[i * MAX_FLAG_LENGTH + j] = j - flag.length();
            }
        }
        // NOTE: This is not secure *at all* but enough to obfuscate the data if
        //       someone decides to read the flash memory and look for strings.
        uint8_t key = 42;
        for (auto &byte : result) {
            byte ^= key;
            key = key ^ key << 3 ^ byte >> 3;
        }
        return result;
    }

    static const auto OBFUSCATED_FLAGS = obfuscate_flags();

    static void foreach_flag(const auto &action) {
        uint8_t key = 42;
        size_t idx = 0;
        char buffer[MAX_FLAG_LENGTH + 1];
        for (int flag = 0; flag < FLAG_COUNT; flag++) {
            for (int i = 0; i < MAX_FLAG_LENGTH; i++) {
                assert(idx < OBFUSCATED_FLAGS.size());
                auto byte = OBFUSCATED_FLAGS[idx];
                buffer[i] = byte ^ key;
                key = key ^ key << 3 ^ byte >> 3;
                idx++;
            }
            buffer[MAX_FLAG_LENGTH] = 0;
            const std::string plaintext(buffer);
            action(static_cast<Flag>(flag), plaintext);
        }
    }

    static std::string get_flag(Flag flag) {
        std::string result;
        foreach_flag([&](Flag f, const std::string &text) {
            if (f == flag)
                result = text;
        });
        return result;
    }

    Flag validate_flag(const std::string &flag) {
        Flag result = INVALID;
        foreach_flag([&](Flag f, const std::string &text) {
            if (text == flag)
                result = f;
        });
        return result;
    }

    bool has_flag(Flag flag) { return false; }

    int count_flags() {
        int count = 0;
        for (int i = 0; i < FLAG_COUNT; i++)
            if (has_flag(static_cast<Flag>(i)))
                count++;
        return count;
    }

    std::string get_konami_code() { return get_flag(BADGE_KONAMI); }

    const image::Image &get_flag_image(Flag flag) {
        switch (flag) {
        case BADGE_README: return image::flag_badge_readme;
        case BADGE_HIDDEN: return image::flag_badge_hidden;
        case BADGE_KONAMI: return image::flag_badge_konami;
        case BADGE_RICKROLL: return image::flag_badge_rickroll;
        case BADGE_PI: return image::flag_badge_pi;
        case BADGE_BAUDOT: return image::flag_badge_baudot;
        case MISC_REBEKAH: return image::flag_misc_rebekah;
        case MISC_SOCIAL: return image::flag_misc_social;
        case MISC_LITERAL1: return image::flag_misc_literal1;
        case MISC_LITERAL2: return image::flag_misc_literal2;
        case ARDUINO_MORSE: return image::flag_arduino_morse;
        case ARDUINO_SERIAL: return image::flag_arduino_serial;
        case CRYPTO_CAESAR: return image::flag_crypto_caesar;
        case LOCKPICK_BASIC: return image::flag_lockpick_basic;
        case LOCKPICK_ELITE: return image::flag_lockpick_elite;
        default: return image::flag_invalid;
        }
    }

} // namespace flags
