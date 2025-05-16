#pragma once

#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <span>
#include <vector>

namespace utils
{

    using sha1_t = std::array<uint8_t, 20>;

    constexpr sha1_t sha1_digest(std::span<const uint8_t> data) {

        // C.f. https://en.wikipedia.org/wiki/SHA-1

        std::array<uint32_t, 5> h = {
            0x67452301,
            0xEFCDAB89,
            0x98BADCFE,
            0x10325476,
            0xC3D2E1F0,
        };

        std::vector<uint8_t> padded_data{data.begin(), data.end()};
        padded_data.push_back(0x80);
        while (padded_data.size() % 64 != 56)
            padded_data.push_back(0);
        const uint64_t ml = data.size() * 8;
        for (int i = 56; i >= 0; i -= 8)
            padded_data.push_back((ml >> i) & 0xff);

        assert(padded_data.size() % 64 == 0);

        for (size_t i0 = 0; i0 < padded_data.size(); i0 += 64) {

            std::array<uint32_t, 80> w = {};

            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 4; j++) {
                    w[i] = (w[i] << 8) | padded_data[i0 + i * 4 + j];
                }
            }

            for (int i = 16; i < 80; i++) {
                w[i] = std::rotl<uint32_t>(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
            }

            uint32_t a = h[0];
            uint32_t b = h[1];
            uint32_t c = h[2];
            uint32_t d = h[3];
            uint32_t e = h[4];
            uint32_t f;
            uint32_t k;

            for (int i = 0; i < 80; i++) {
                if (i < 20) {
                    f = (b & c) | (~b & d);
                    k = 0x5A827999;
                }
                else if (i < 40) {
                    f = b ^ c ^ d;
                    k = 0x6ED9EBA1;
                }
                else if (i < 60) {
                    f = (b & c) ^ (b & d) ^ (c & d);
                    k = 0x8F1BBCDC;
                }
                else {
                    f = b ^ c ^ d;
                    k = 0xCA62C1D6;
                }

                const uint32_t temp = std::rotl<uint32_t>(a, 5) + f + e + k + w[i];
                e = d;
                d = c;
                c = std::rotl<uint32_t>(b, 30);
                b = a;
                a = temp;
            }

            h[0] += a;
            h[1] += b;
            h[2] += c;
            h[3] += d;
            h[4] += e;

        }

        sha1_t result;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 4; j++) {
                result[i * 4 + j] = (h[i] >> ((3 - j) * 8)) & 0xFF;
            }
        }

        return result;

    }

    constexpr sha1_t sha1_digest(const std::string& text) {
        std::vector<uint8_t> data(text.begin(), text.end());
        return sha1_digest(data);
    }

    constexpr std::string sha1_hex_string(const auto& data) {
        constexpr auto HEX_ALPHABET = "0123456789abcdef";
        const auto hash = sha1_digest(data);
        char buffer[41];
        for (int i = 0; i < 20; i++) {
            buffer[i * 2 + 0] = HEX_ALPHABET[hash[i] >> 4];
            buffer[i * 2 + 1] = HEX_ALPHABET[hash[i] & 0xF];
        }
        buffer[40] = 0;
        return {buffer};
    }

    static_assert(sha1_hex_string("") == "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    static_assert(sha1_hex_string("The quick brown fox jumps over the lazy dog") == "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12");

}
