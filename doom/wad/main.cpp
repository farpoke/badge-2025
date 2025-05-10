
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "wad.hpp"


bool compare_data(const wad::byte_vector& a, const wad::byte_vector& b) {
    if (a.size() != b.size())
        return false;
    for (int i = 0; i < a.size(); i++)
        if (a[i] != b[i])
            return false;
    return true;
}


float mib(auto n) {
    return static_cast<float>(n) / (1024 * 1024);
}


void run(std::string_view input, std::string_view output) {
    const auto input_path = std::filesystem::absolute(input);
    const auto output_path = std::filesystem::absolute(output);

    printf("- Input:  %s\n", input_path.generic_string().c_str());
    printf("- Output: %s\n", output_path.generic_string().c_str());

    if (!std::filesystem::exists(input_path)) {
        throw std::runtime_error("Input file not found");
    }

    std::ifstream input_file(input_path, std::ios::binary);
    if (!input_file.is_open()) {
        throw std::runtime_error("Failed to open input file");
    }
    const wad::byte_vector input_bytes{
        std::istreambuf_iterator<char>(input_file),
        std::istreambuf_iterator<char>()
    };
    input_file.close();

    printf("- Read %.2f MiB from input\n", mib(input_bytes.size()));

    wad::WadFile wad_file(input_bytes);

    const auto reserialized_bytes = wad_file.serialize();

#if 0
    std::ofstream reserialized_file("reserialized.wad", std::ios::binary | std::ios::out);
    reserialized_file.write(reinterpret_cast<const char*>(reserialized_bytes.data()), reserialized_bytes.size());
    reserialized_file.close();
    printf("- Wrote %.2f MiB to reserialized.wad\n", static_cast<float>(input_bytes.size()) / 1024 / 1024);
#endif

    if (compare_data(input_bytes, reserialized_bytes)) {
        printf("- Reserialized successfully\n");
    }
    else {
        printf("! Reserialization differs from original input\n");
        if (input_bytes.size() != reserialized_bytes.size()) {
            printf("  input size        : %llu\n", input_bytes.size());
            printf("  reserialized size : %llu\n", reserialized_bytes.size());
        }
        else {
            for (int i = 0; i < input_bytes.size(); i++) {
                if (input_bytes[i] != reserialized_bytes[i])
                    printf("  [%d] %02x != %02x\n", i, reserialized_bytes[i], input_bytes[i]);
            }
        }
        exit(3);
    }

    wad_file.remove_audio();

    const auto original_without_audio = wad_file.serialize();
    printf(
        "- Serialized to %.2f MiB (%lld%%) with audio removed\n",
        mib(original_without_audio.size()),
        original_without_audio.size() * 100 / input_bytes.size()
    );

    wad_file.report_sizes();

    wad_file.compress();

    const auto compressed_bytes = wad_file.serialize();

    printf(
        "- Compressed to %.2f MiB (%llu%%)\n",
        mib(compressed_bytes.size()),
        compressed_bytes.size() * 100 / original_without_audio.size()
    );

    wad_file.report_sizes();
}

int main(const int argc, char *argv[]) {
    if (argc != 3) {
        const auto program = std::filesystem::path(argv[0]);
        printf("Usage: %s input.wad output.twd\n", program.filename().generic_string().c_str());
        return 1;
    }

    try {
        run(argv[1], argv[2]);
    } catch (const std::exception &e) {
        printf("\nEXCEPTION\nstd::exception : %s\n", e.what());
        return 2;
    }
    catch (...) {
        printf("\nEXCEPTION\n...\n");
        return 2;
    }

    return 0;
}
