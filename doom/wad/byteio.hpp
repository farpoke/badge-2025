#pragma once

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace wad {

    struct ByteWriter {
        std::vector<uint8_t> data;

        void write_u8(uint8_t value) {
            data.emplace_back(value);
        }

        void write_u16(uint16_t value) {
            data.emplace_back(value & 0xFF);
            data.emplace_back((value >> 8) & 0xFF);
        }

        void write_u32(uint32_t value) {
            data.emplace_back(value & 0xFF);
            data.emplace_back((value >> 8) & 0xFF);
            data.emplace_back((value >> 16) & 0xFF);
            data.emplace_back((value >> 24) & 0xFF);
        }

        void write_i8(int8_t value) {
            data.emplace_back(value);
        }

        void write_i16(int16_t value) {
            data.emplace_back(value & 0xFF);
            data.emplace_back((value >> 8) & 0xFF);
        }

        void write_i32(int32_t value) {
            data.emplace_back(value & 0xFF);
            data.emplace_back((value >> 8) & 0xFF);
            data.emplace_back((value >> 16) & 0xFF);
            data.emplace_back((value >> 24) & 0xFF);
        }

        template<int N>
        void write_text(std::string_view text) {
            for (int i = 0; i < N; i++) {
                data.emplace_back(text.length() > i ? text[i] : 0);
            }
        }

        void write_bytes(std::span<const uint8_t> bytes) {
            data.insert(data.end(), bytes.begin(), bytes.end());
        }

        void write_struct(const auto& obj) {
            write_bytes(std::span(reinterpret_cast<const uint8_t*>(&obj), sizeof(obj)));
        }

        void write_collection(const auto& collection) {
            for (const auto& item : collection)
                write_struct(item);
        }
    };

}
