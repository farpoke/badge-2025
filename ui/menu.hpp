#pragma once

#include "state.hpp"

#include <string>
#include <vector>

namespace ui
{

    class Menu final : public State {
    public:
        typedef void (*Callback)();

        void add_item(std::string_view label, const StatePtr &target_state);
        void add_item(std::string_view label, Callback callback);

        void update(int delta_ms) override;
        void draw() override;

        bool is_main = false;

    protected:
        struct Item {
            Item()             = default;
            Item(const Item &) = delete;
            Item(Item &&)      = default;
            ~Item()            = default;

            Item &operator=(const Item &) = delete;
            Item &operator=(Item &&)      = default;

            Item(std::string_view label, const StatePtr &target_state) : label(label), target_state(target_state) {}
            Item(std::string_view label, Callback callback) : label(label), callback(callback) {}

            std::string label        = {};
            StatePtr    target_state = nullptr;
            Callback    callback     = nullptr;
        };

        std::vector<Item> items;
        int selected_item = 0;
        int current_offset = 0;
        int target_offset = 0;
    };

} // namespace ui
