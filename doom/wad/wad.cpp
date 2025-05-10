#include "wad.hpp"

#include <cassert>
#include <map>
#include <set>

#include "byteio.hpp"

#define MAKE_12_12_BYTES(first, second) \
    static_cast<uint8_t>((first) & 0xFF), \
    static_cast<uint8_t>((((first) & 0xF00) >> 4) | ((second) & 0xF)), \
    static_cast<uint8_t>(((second) & 0xFF00) >> 4)

namespace wad {

    LumpType guess_type(std::string_view name) {
        using enum LumpType;
        if (name == "ENDOOM" ||
            name == "PLAYPAL" ||
            name == "COLORMAP" ||
            name == "TEXTURE1" ||
            name == "TEXTURE2" ||
            name == "PNAMES")
            return MISC;
        else if (name.starts_with("DS") ||
                 name.starts_with("DP") ||
                 name.starts_with("D_") ||
                 name == "GENMIDI" ||
                 name == "DMXGUS")
            return AUDIO;
        else if (name.starts_with("WI") ||
                 name.starts_with("M_") ||
                 name.starts_with("ST") ||
                 name.starts_with("BRDR") ||
                 name.starts_with("AMMNUM") ||
                 name.starts_with("END") ||
                 name == "HELP1" ||
                 name == "CREDIT" ||
                 name == "VICTORY2" ||
                 name == "TITLEPIC" ||
                 name == "INTERPIC" ||
                 name.starts_with("PFUB"))
            return PICTURE;
        else if (name.starts_with("DEMO"))
            return DEMO;
        else if (name == "THINGS")
            return THINGS;
        else if (name == "LINEDEFS")
            return LINEDEFS;
        else if (name == "SIDEDEFS")
            return SIDEDEFS;
        else if (name == "VERTEXES")
            return VERTEXES;
        else if (name == "SEGS")
            return SEGS;
        else if (name == "SSECTORS")
            return SSECTORS;
        else if (name == "NODES")
            return NODES;
        else if (name == "SECTORS")
            return SECTORS;
        else if (name == "REJECT")
            return REJECT;
        else if (name == "BLOCKMAP")
            return BLOCKMAP;
        else
            return UNKNOWN;
    }

    void Lump::process(WadFile *wad) {
    }

    void Lump::compress(WadFile* wad) {
    }

    void Lump::decompress(WadFile* wad) {
    }

    LumpPtr make_lump(std::string name, LumpType type, byte_vector data) {
        switch (type) {
            using enum LumpType;
            case VERTEXES: return std::make_shared<VertexesLump>(std::move(name), type, std::move(data));
            case LINEDEFS: return std::make_shared<LineDefsLump>(std::move(name), type, std::move(data));
            case SIDEDEFS: return std::make_shared<SideDefsLump>(std::move(name), type, std::move(data));
            case PICTURE: return std::make_shared<PictureLump>(std::move(name), type, std::move(data));
            default:
                return std::make_shared<Lump>(std::move(name), type, std::move(data));
        }
    }

    void VertexesLump::process(WadFile* wad) {
        Lump::process(wad);

        assert(data.size() % 4 == 0);
        vertices.resize(data.size() / 4);
        memcpy(vertices.data(), data.data(), data.size());
    }

    void LineDefsLump::process(WadFile* wad) {
        Lump::process(wad);

        assert(data.size() % sizeof(linedef_t) == 0);
        linedefs.resize(data.size() / sizeof(linedef_t));
        memcpy(linedefs.data(), data.data(), data.size());
    }

    void LineDefsLump::compress(WadFile* wad) {
#if 0
        printf("\n----- LINEDEFS -----\n");
        for (const auto& def : linedefs) {
            printf(
                "%4d %4d %02x %02x %02x %4d %4d\n",
                def.v1,
                def.v2,
                def.flags,
                def.special,
                def.tag,
                def.sidenum[0],
                def.sidenum[1]
            );
        }
        printf("\n");
#endif
    }

    void LineDefsLump::decompress(WadFile* wad) {
    }

    void SideDefsLump::process(WadFile* wad) {
        Lump::process(wad);

        assert(data.size() % sizeof(sidedef_t) == 0);
        sidedefs.resize(data.size() / sizeof(sidedef_t));
        const auto* src = reinterpret_cast<const sidedef_t*>(data.data());
        for (int i = 0; i < sidedefs.size(); i++) {
            sidedefs[i].x_offset = src[i].x_offset;
            sidedefs[i].y_offset = src[i].y_offset;
            sidedefs[i].upper_texture = wad_text_to_string(src[i].upper_texture);
            sidedefs[i].lower_texture = wad_text_to_string(src[i].lower_texture);
            sidedefs[i].middle_texture = wad_text_to_string(src[i].middle_texture);
            sidedefs[i].facing_sector = src[i].facing_sector;

            wad->register_texture_name(sidedefs[i].upper_texture);
            wad->register_texture_name(sidedefs[i].lower_texture);
            wad->register_texture_name(sidedefs[i].middle_texture);
        }
    }

    void SideDefsLump::compress(WadFile* wad) {
        ByteWriter writer;
        for (const auto& def : sidedefs) {
            assert(def.x_offset >= -2048 && def.x_offset < 2048);
            assert(def.y_offset >= -2048 && def.y_offset < 2048);
            const auto upper = wad->get_texture_id(def.upper_texture);
            const auto lower = wad->get_texture_id(def.lower_texture);
            const auto middle = wad->get_texture_id(def.middle_texture);
            assert(upper < 2048);
            assert(lower < 2048);
            assert(middle < 2048);
            assert(def.facing_sector < 2048);
            uint8_t bytes[9] = {
                MAKE_12_12_BYTES(def.x_offset, def.y_offset),
                MAKE_12_12_BYTES(upper, lower),
                MAKE_12_12_BYTES(middle, def.facing_sector),
            };
            writer.write_bytes(bytes);
        }

        const auto original_size = data.size();

        data = writer.data;

        const auto new_size = data.size();

        // printf("? SIDEDEFS   %5llu -> %5llu   %3llu%%\n", original_size, new_size, new_size * 100 / original_size);
    }

    void SideDefsLump::decompress(WadFile* wad) {
    }

    void PictureLump::process(WadFile *wad) {
        const auto header = *reinterpret_cast<const patch_header_t*>(data.data());
        width = header.width;
        height = header.height;
        left_offset = header.left_offset;
        top_offset = header.top_offset;
#if 0
        const auto kib = static_cast<float>(data.size()) / 1024;
        if (kib > 1)
            printf("? PICTURE   %-8s   %3d x %3d   %4.1f KiB\n", name.c_str(), width, height, kib);
#endif
    }

    void PictureLump::compress(WadFile *wad) {
        if (width == 320 && height == 200) {
            data.resize(data.size() / 4);
        }
    }

    void PictureLump::decompress(WadFile *wad) {
    }

    WadFile::WadFile(const byte_vector &data) {
        assert(data.size() >= sizeof(wadinfo_t));
        const auto* ptr = data.data();
        const auto file_header = *reinterpret_cast<const wadinfo_t*>(ptr);

        identifier = wad_text_to_string(file_header.identification);
        assert(identifier == "IWAD" || identifier == "PWAD");

        assert(file_header.infotableof >= sizeof(wadinfo_t));
        assert(file_header.numlumps >= 0);
        assert(file_header.infotableof + file_header.numlumps * sizeof(filelump_t) == data.size());

        const auto* directory = reinterpret_cast<const filelump_t*>(data.data() + file_header.infotableof);
        int expected_offset = sizeof(wadinfo_t);
        bool is_flat = false;
        bool is_sprite = false;
        bool is_patch = false;
        for (int i = 0; i < file_header.numlumps; i++) {
            const auto entry = directory[i];
            assert(entry.size == 0 || entry.filepos >= sizeof(wadinfo_t));
            assert(entry.filepos + entry.size <= file_header.infotableof);
            const auto entry_data = byte_vector(ptr + entry.filepos, ptr + entry.filepos + entry.size);
            const auto name = wad_text_to_string(entry.name);
            LumpType type = LumpType::UNKNOWN;
            if (entry.size == 0)
                type = LumpType::MARKER;
            else if (is_flat)
                type = LumpType::FLAT;
            else if (is_sprite)
                type = LumpType::SPRITE;
            else if (is_patch)
                type = LumpType::PATCH;
            else
                type = guess_type(name);
            auto lump = make_lump(name, type, entry_data);
            lump->process(this);
            lumps.push_back(lump);
            if (entry.size == 0) {
                if (name == "F_START") is_flat = true;
                else if (name == "F_END") is_flat = false;
                else if (name == "S_START") is_sprite = true;
                else if (name == "S_END") is_sprite = false;
                else if (name == "P_START") is_patch = true;
                else if (name == "P_END") is_patch = false;
            }
            if (lump->type == LumpType::UNKNOWN) {
                printf("? Unknown lump %s\n", name.c_str());
            }
            if (entry.filepos > 0) {
                assert(entry.filepos == expected_offset);
                expected_offset = entry.filepos + entry.size;
                while ((expected_offset % 4) != 0) {
                    assert(data[expected_offset] == data[entry.filepos]);
                    expected_offset++;
                }
            }
        }
    }

    void WadFile::report_sizes() const {
        printf("\n----- lumps -----\n");
        std::map<LumpType, size_t> by_type;
        for (const auto& lump : lumps) {
            by_type[lump->type] += lump->data.size();
        }
#define TYPE(NAME) if (by_type[LumpType::NAME] > 0) printf("%-8s : %5d KiB\n", #NAME, static_cast<int>(by_type[LumpType::NAME] / 1024));
        TYPE(PICTURE)
        TYPE(SPRITE)
        TYPE(FLAT)
        TYPE(PATCH)
        TYPE(AUDIO)
        TYPE(THINGS)
        TYPE(LINEDEFS)
        TYPE(SIDEDEFS)
        TYPE(VERTEXES)
        TYPE(SEGS)
        TYPE(SSECTORS)
        TYPE(NODES)
        TYPE(SECTORS)
        TYPE(REJECT)
        TYPE(BLOCKMAP)
        TYPE(MISC)
        TYPE(DEMO)
        TYPE(UNKNOWN)
#undef TYPE
        printf("\n");
    }


    byte_vector WadFile::serialize() const {
        ByteWriter writer;

        const int header_size = sizeof(wadinfo_t);

        int n_lumps = 0;
        size_t total_lump_size = 0;
        for (const auto& lump : lumps) {
            n_lumps++;
            total_lump_size += lump->data.size();
            if (total_lump_size % 4 != 0)
                total_lump_size += 4 - (total_lump_size % 4);
        }

        wadinfo_t header = {};
        memcpy(header.identification, identifier.c_str(), sizeof(header.identification));
        header.numlumps = static_cast<int>(lumps.size());
        header.infotableof = static_cast<int>(header_size + total_lump_size);
        writer.write_struct(header);

        int current_pos = sizeof(header);
        std::vector<filelump_t> directory(n_lumps);
        memset(directory.data(), 0, n_lumps * sizeof(filelump_t));

        int dir_index = 0;
        for (const auto& lump : lumps) {
            const auto size = static_cast<int>(lump->data.size());
            if (size == 0 && (lump->name.ends_with("_START") || lump->name.ends_with("_END")))
                directory[dir_index].filepos = 0;
            else
                directory[dir_index].filepos = current_pos;
            directory[dir_index].size = size;
            memcpy(directory[dir_index].name, lump->name.c_str(), std::min(lump->name.length(), sizeof(directory[dir_index].name)));
            writer.write_bytes(lump->data);
            current_pos += size;
            while (current_pos % 4 != 0) {
                writer.write_u8(lump->data[0]);
                current_pos++;
            }
            dir_index++;
        }

        assert(current_pos == header.infotableof);
        assert(dir_index == n_lumps);

        writer.write_collection(directory);

        if (is_compressed) {
            const auto n = static_cast<int>(texture_id_to_name.size());
            writer.write_u16(n);
            for (int i = 0; i < n; i++) {
                writer.write_text<8>(texture_id_to_name.at(i));
            }
        }

        return writer.data;
    }

    void WadFile::remove_audio() {
        int idx = 0;
        while (idx < lumps.size()) {
            if (lumps[idx]->type == LumpType::AUDIO) {
                lumps.erase(lumps.begin() + idx);
            }
            else {
                idx++;
            }
        }
    }

    void WadFile::compress() {
        assert(!is_compressed);
        for (const auto& lump : lumps) {
            lump->compress(this);
        }
        is_compressed = true;
    }

    void WadFile::decompress() {
        assert(is_compressed);
        for (const auto& lump : lumps) {
            lump->decompress(this);
        }
        is_compressed = false;
    }

    void WadFile::register_texture_name(const std::string& name) {
        assert(name.length() <= 8);
        if (texture_name_to_id.contains(name))
            return;
        const auto id = static_cast<uint16_t>(texture_name_to_id.size());
        texture_name_to_id[name] = id;
        texture_id_to_name[id] = name;
    }

    [[nodiscard]] uint16_t WadFile::get_texture_id(const std::string& name) const {
        if (name == "-")
            return 0;
        else
            return texture_name_to_id.at(name);
    }

    [[nodiscard]] std::string WadFile::get_texture_name(uint16_t id) const {
        if (id == 0)
            return "-";
        else
            return texture_id_to_name.at(id);
    }

}
