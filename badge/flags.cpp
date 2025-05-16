#include "flags.hpp"

#include <array>
#include <assets.hpp>
#include <cassert>
#include <cstdint>
#include <cstring>

#include <pico/stdio.h>

#include <badge/storage.hpp>
#include <utils/sha1.hpp>

namespace flags
{

#include "flags-data.inc"

    consteval auto compute_digests() {
        std::array<utils::sha1_t, FLAG_COUNT> digests = {};
        for (auto i = 0; i < FLAG_COUNT; i++) {
            digests[i] = utils::sha1_digest(get_plaintext_flag(static_cast<Flag>(i)));
        }
        return digests;
    }

    static const auto FLAG_DIGESTS __attribute__((used)) = compute_digests();

    static std::vector<Flag> _foundFlags = {};
    static size_t _nextStorageIndex = 0;

    static bool has_flag(Flag flag) {
        for (auto f : _foundFlags) {
            if (flag == f)
                return true;
        }
        return false;
    }

    static Flag validate_flag(const std::string &text) {
        const auto digest = utils::sha1_digest(text);
        for (auto i = 0; i < FLAG_COUNT; i++) {
            if (FLAG_DIGESTS[i] == digest)
                return static_cast<Flag>(i);
        }
        return INVALID;
    }

    void init() {
        printf("> Loading flags from storage...\n");
        // Reset our flag cache.
        _foundFlags.clear();
        // Go through stored flags and collect valid ones.
        std::vector<std::string> valid_flags;
        size_t idx = 0;
        while (idx < storage::FLAG_STORAGE_SIZE) {
            const auto n = strnlen(storage::ram_data->entered_flags + idx, storage::FLAG_STORAGE_SIZE - idx);
            if (n == 0)
                break;
            const auto text = std::string(storage::ram_data->entered_flags + idx, n);
            const auto flag = validate_flag(text);
            if (flag != INVALID && !has_flag(flag)) {
                printf("  Found flag %d\n", flag);
                _foundFlags.push_back(flag);
                valid_flags.push_back(text);
            }
            idx += n + 1;
        }
        // Clear and write back all valid flags to storage.
        memset(storage::ram_data->entered_flags, 0, storage::FLAG_STORAGE_SIZE);
        idx = 0;
        for (const auto& flag : valid_flags) {
            memcpy(storage::ram_data->entered_flags + idx, flag.c_str(), flag.size());
            idx += flag.size() + 1;
        }
        assert(idx <= storage::FLAG_STORAGE_SIZE);
        // Store the index we ended at, for when we want to add more entered flags to storage.
        _nextStorageIndex = idx;
        // Save the possibly updated flags. If we ended up with the same stored data we started with, then
        // this function will simply return without doing unnecessary FLASH writes.
        storage::save();
    }

    Flag enter_flag(const std::string &text) {
        printf("> Enter flag \"%s\" ...\n", text.c_str());
        const auto flag = validate_flag(text);
        if (flag == INVALID) {
            printf("  Invalid\n");
            return INVALID;
        }
        assert(flag >= 0 && flag < FLAG_COUNT);
        if (has_flag(flag)) {
            printf("  Already entered\n");
            return flag;
        }
        printf("  Accepting new flag %d\n", flag);
        _foundFlags.push_back(flag);
        memcpy(storage::ram_data->entered_flags + _nextStorageIndex, text.c_str(), text.size());
        _nextStorageIndex += text.size() + 1;
        assert(_nextStorageIndex <= storage::FLAG_STORAGE_SIZE);
        storage::save();
        return flag;
    }

    const std::vector<Flag> &get_found_flags() {
        return _foundFlags;
    }

    std::string get_konami_code() {
        // The konami flag is special because we need to print it ourselves.
        static constexpr auto KONAMI = get_plaintext_flag(BADGE_KONAMI);
        return KONAMI;
    }

    const image::Image &get_flag_image(Flag flag) {
        switch (flag) {
        case BADGE_README: return image::flag_badge_readme;
        case BADGE_HIDDEN: return image::flag_badge_hidden;
        case BADGE_KONAMI: return image::flag_badge_konami;
        case BADGE_RICKROLL: return image::flag_badge_rickroll;
        case BADGE_PI: return image::flag_badge_pi;
        case BADGE_BAUDOT: return image::flag_badge_baudot;
        case MISC_REBEKAH: return image::flag_misc_rebekah;
        case MISC_LITERAL1: return image::flag_misc_literal1;
        case MISC_LITERAL2: return image::flag_misc_literal2;
        case MISC_MVP: return image::flag_mvp;
        case ARDUINO_MORSE: return image::flag_arduino_morse;
        case ARDUINO_SERIAL: return image::flag_arduino_serial;
        case CRYPTO_CAESAR: return image::flag_crypto_caesar;
        case LOCKPICK_BASIC: return image::flag_lockpick_basic;
        case LOCKPICK_ELITE: return image::flag_lockpick_elite;
        case LOCKPICK_DIY: return image::flag_lockpick_diy;
        case WEB_MEDIUM: return image::flag_web;
        case PWN_MEDIUM: return image::flag_pwn_medium;
        case PWN_ELITE: return image::flag_pwn_elite;
        case RE_EASY: return image::flag_re_easy;
        case RE_MEDIUM: return image::flag_re_medium;
        case RE_ELITE: return image::flag_re_elite;
        case STEGO_EASY: return image::flag_stego_easy;
        case STEGO_ELITE: return image::flag_stego_elite;
        case BASIC_2024_HASH: return image::flag_hash_easy;
        case BASIC_2024_LOCK: return image::flag_lockpick_basic;
        case BASIC_2024_CRYPTO: return image::flag_old_crypto;
        case BASIC_2024_CRED: return image::flag_cred_1;
        case ELITE_2024_HASH: return image::flag_hash_elite;
        case ELITE_2024_SOCIAL: return image::flag_misc_social;
        case ELITE_2024_CRED: return image::flag_cred_2;
        case EXPLORER_1: return image::flag_explorer_1;
        case EXPLORER_2: return image::flag_explorer_2;
        case EXPLORER_3: return image::flag_explorer_3;
        default: return image::red_x;
        }
    }

} // namespace flags
