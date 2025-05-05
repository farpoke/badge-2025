#include "readme.hpp"

#include <cstdio>

#include <badge/buttons.hpp>
#include <badge/drawing.hpp>
#include <badge/font.hpp>
#include <fs/fs.hpp>
#include <ui/ui.hpp>

namespace ui
{

    void Readme::update(int delta_ms) {
        State::update(delta_ms);
        if (buttons::b())
            pop_state();
        else if (buttons::up_current()) {
            scroll -= delta_ms / 10;
            if (scroll < 0)
                scroll = 0;
            is_scrolling = true;
        }
        else if (buttons::down_current()) {
            scroll += delta_ms / 10;
            if (scroll > max_scroll)
                scroll = max_scroll;
            is_scrolling = true;
        }
        else {
            is_scrolling = false;
        }
    }

    void Readme::draw() {
        drawing::clear(COLOR_BLACK);

        int x = padding;
        int y = padding + line_height - scroll;

        for (const auto &line : lines) {
            const auto render = font->render(line);
            if ((y + render.dy) > lcd::HEIGHT)
                break;
            if ((y + render.dy + render.height) >= 0)
                drawing::draw_text(x, y, 0, 0, COLOR_WHITE, render);
            y += line_height;
        }

        if (is_scrolling && max_scroll > 0) {
            const int bar_height = line_height;
            const int bar_space = lcd::HEIGHT - bar_height;
            const int bar_offset = bar_space * scroll / max_scroll;
            drawing::fill_rect(lcd::WIDTH - 4, bar_offset, 2, bar_height, rgb24(0xFF8800));
        }
    }

    void Readme::pause() {
        // When we pause, it should be because we've been popped, so try to free all used memory.
        lines = {};
    }

    void Readme::resume() {
        if (font == nullptr) {
            font = &font::noto_sans;
            // We guess that an apostrophe and a `g` are representative of the highest ascent and lowest descent,
            // and use that to compute the line height/spacing.
            const auto measure = font->measure("'g");
            line_height        = measure.bottom - measure.top + 2;
        }

        scroll = 0;

        int available_width = lcd::WIDTH - padding * 2;

        // Fetch the contents of the README.txt file.
        const auto readme_bytes = fs::get_file_span("README  TXT");
        const auto readme_text =
                std::string_view(reinterpret_cast<const char *>(readme_bytes.data()), readme_bytes.size());

        // Iterate through the text and produce wrapped lines suitable for drawing.
        // `line` here refers to these wrapped lines for the output, not input lines.
        auto line_start = readme_text.begin();
        while (line_start < readme_text.end()) {

            // First, skip any leading spaces in the line.
            if (*line_start == ' ') {
                line_start++;
                continue;
            }

            // If we start with a line feed (i.e. a blank line), then add a blank line and continue.
            if (*line_start == '\n') {
                lines.emplace_back(line_start, line_start + 1);
                line_start++;
                // Compress two blank lines into one (mainly to reduce extra scrolling).
                if (line_start < readme_text.end() && *line_start == '\n')
                    line_start++;
                continue;
            }

            // Now, try to add successive words to the line without going past the available horizontal space.
            auto line_end = line_start;
            while (line_end < readme_text.end()) {
                auto current = line_end + 1;
                while (current < readme_text.end() && *current != ' ' && *current != '\n') {
                    current++;
                }
                const auto measure = font->measure(std::string_view(line_start, current));
                if (measure.right > available_width)
                    break;
                line_end = current;
                if ((current + 1) < readme_text.end() && *current == '\n' && *(current + 1) == '\n')
                    break;
            }

            // If we completely failed to build the line (due to very long word), just try and add as much as possible.
            if (line_end == line_start) {
                while (line_end < readme_text.end() && *line_end != ' ' && *line_end != '\n') {
                    line_end++;
                    const auto measure = font->measure(std::string_view(line_start, line_end));
                    if (measure.right > available_width)
                        break;
                }
            }

            // Absorb trailing whitespace and possibly up to one line feed.
            while (line_end < readme_text.end()) {
                if (*line_end == ' ') {
                    line_end++;
                }
                else if (*line_end == '\n') {
                    line_end++;
                    break;
                }
                else {
                    break;
                }
            }

            // Add our new line to the list and continue with the next one.
            lines.emplace_back(line_start, line_end);
            if (line_start == line_end && line_end != readme_text.end())
                line_start = line_end + 1; // Failsafe to prevent accidental infinite loop.
            else
                line_start = line_end;
        }

        max_scroll = lines.size() * line_height + line_height - lcd::HEIGHT;
        if (max_scroll < 0)
            max_scroll = 0;
    }

} // namespace ui
