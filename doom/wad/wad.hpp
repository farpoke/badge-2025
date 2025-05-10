#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace wad {

    struct wadinfo_t {
        char identification[4];
        int32_t numlumps;
        int32_t infotableof;
    };

    static_assert(sizeof(wadinfo_t) == 12);

    struct filelump_t {
        int32_t filepos;
        int32_t size;
        char name[8];
    };

    static_assert(sizeof(filelump_t) == 16);

    struct vertex_t {
        int16_t x;
        int16_t y;
    };

    static_assert(sizeof(vertex_t) == 4);

    struct linedef_t {
        int16_t v1;
        int16_t v2;
        int16_t flags;
        int16_t special;
        int16_t tag;
        int16_t sidenum[2];
    };

    static_assert(sizeof(linedef_t) == 14);

    struct sidedef_t {
        int16_t x_offset;
        int16_t y_offset;
        char upper_texture[8];
        char lower_texture[8];
        char middle_texture[8];
        int16_t facing_sector;
    };

    static_assert(sizeof(sidedef_t) == 30);

    using byte_vector = std::vector<uint8_t>;

    template<int N>
    std::string wad_text_to_string(const char (&text)[N]) {
        const auto len = strnlen(text, N);
        return std::string(text, len);
    }

    enum class LumpType {
        UNKNOWN = 0,
        MARKER,
        PICTURE,
        SPRITE,
        FLAT,
        PATCH,
        AUDIO,
        THINGS,
        LINEDEFS,
        SIDEDEFS,
        VERTEXES,
        SEGS,
        SSECTORS,
        NODES,
        SECTORS,
        REJECT,
        BLOCKMAP,
        DEMO,
        MISC,
    };

    class WadFile;

    struct Lump {
        std::string name = {};
        LumpType type = {};
        byte_vector data = {};

        Lump() = default;
        virtual ~Lump() = default;

        Lump(std::string name, LumpType type, byte_vector data)
        : name(std::move(name)), type(type), data(std::move(data)) {}

        virtual void process(WadFile* wad);
        virtual void compress(WadFile* wad);
        virtual void decompress(WadFile* wad);
    };

    using LumpPtr = std::shared_ptr<Lump>;

    LumpPtr make_lump(std::string name, LumpType type, byte_vector data);

    struct VertexesLump : Lump {
        std::vector<vertex_t> vertices = {};

        using Lump::Lump;

        void process(WadFile* wad) override;
    };

    struct LineDefsLump : Lump {
        std::vector<linedef_t> linedefs = {};

        using Lump::Lump;

        void process(WadFile* wad) override;
        void compress(WadFile* wad) override;
        void decompress(WadFile* wad) override;
    };

    struct SideDef {
        int16_t x_offset;
        int16_t y_offset;
        std::string upper_texture;
        std::string lower_texture;
        std::string middle_texture;
        int16_t facing_sector;
    };

    struct SideDefsLump : Lump {
        std::vector<SideDef> sidedefs = {};

        using Lump::Lump;

        void process(WadFile* wad) override;
        void compress(WadFile* wad) override;
        void decompress(WadFile* wad) override;
    };

    struct PaletteEntry {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    static_assert(sizeof(PaletteEntry) == 3);

    struct Palette {
        std::array<PaletteEntry, 256> colors;
    };

    static_assert(sizeof(Palette) == 256 * 3);

    struct patch_header_t {
        uint16_t width;
        uint16_t height;
        uint16_t left_offset;
        uint16_t top_offset;
        uint32_t column_offsets[];
    };

    static_assert(sizeof(patch_header_t) == 8);

    struct PictureLump : Lump {
        int width;
        int height;
        int left_offset;
        int top_offset;

        using Lump::Lump;

        void process(WadFile* wad) override;
        void compress(WadFile* wad) override;
        void decompress(WadFile* wad) override;
    };

    class WadFile {
    public:
        explicit WadFile(const byte_vector& data);
        ~WadFile() = default;

        void report_sizes() const;

        [[nodiscard]] byte_vector serialize() const;

        void remove_audio();

        void compress();
        void decompress();

        void register_texture_name(const std::string& name);
        [[nodiscard]] uint16_t get_texture_id(const std::string& name) const;
        [[nodiscard]] std::string get_texture_name(uint16_t id) const;

    private:
        std::string identifier;
        std::vector<LumpPtr> lumps;

        bool is_compressed = false;

        std::map<std::string, int> texture_name_to_id;
        std::map<int, std::string> texture_id_to_name;
    };

}
