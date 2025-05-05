#include "fs.hpp"

#include <cassert>
#include <cstring>

#include <assets.hpp>

namespace fs
{

    struct __packed DirEntry {
        char short_name[11];
        uint8_t attributes;
        uint8_t sfn_case;
        uint8_t create_time_tenths;
        uint16_t create_time;
        uint16_t create_date;
        uint16_t access_date;
        uint16_t cluster_high;
        uint16_t write_time;
        uint16_t write_date;
        uint16_t cluster_low;
        uint32_t size;
    };

    static_assert(sizeof(DirEntry) == 32);

    std::span<const uint8_t> get_file_span(std::string_view short_name) {
        // The root directory entries should be in block 2, so 512*2=1024 bytes into the disk image.
        auto *root_entries = reinterpret_cast<const DirEntry*>(DISK_IMAGE + 1024);
        // A block can fit 16 entries. Iterate over them and see if we find the requested short name.
        for (int i = 0; i < 16; i++) {
            const auto& entry = root_entries[i];
            if (strncmp(short_name.data(), entry.short_name, 11) != 0)
                continue; // This entry was not the one.
            // We should never have a disk large enough to need the upper two bytes of the cluster number.
            assert(entry.cluster_high == 0);
            // Figure out the offset into the disk image, and return a span with the file data.
            // This only works because we generate a read-only image with no fragmentation.
            const auto offset = (entry.cluster_low + 1) * 512;
            return { &DISK_IMAGE[offset], entry.size };
        }
        // Did not find the file; just return an empty span.
        return {};
    }

}
