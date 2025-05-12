#include "flags.hpp"

#include <array>
#include <cassert>
#include <cstdint>

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
        for (auto& byte : result) {
            byte ^= key;
            key = key ^ key << 3 ^ byte >> 3;
        }
        return result;
    }

    static const auto OBFUSCATED_FLAGS = obfuscate_flags();

    static void foreach_flag(const auto& action) {
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
        foreach_flag([&](Flag f, const std::string& text) {
            if (f == flag)
                result = text;
        });
        return result;
    }

    Flag validate_flag(const std::string &flag) {
        Flag result = INVALID;
        foreach_flag([&](Flag f, const std::string& text) {
            if (text == flag)
                result = f;
        });
        return result;
    }

    bool has_flag(Flag flag) {
        return false;
    }

    int count_flags() {
        int count = 0;
        for (int i = 0; i < FLAG_COUNT; i++)
            if (has_flag(static_cast<Flag>(i)))
                count++;
        return count;
    }

    std::string get_konami_code() {
        return get_flag(BADGE_KONAMI);
    }

}
