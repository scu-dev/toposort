#pragma once
#include <array>
#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "toposort.hpp"

namespace Toposort {
    typedef uint8_t u8;
    typedef uint64_t u64;
    using std::array, std::max, std::string, std::stringstream, std::string_view, std::vector;

    inline string printArrangements(const vector<vector<string>>& arrangements) noexcept {
        stringstream result;
        for (u64 i = 0; i < arrangements.size(); i++) {
            result << "第 " << i + 1 << " 学期：";
            for (u64 j = 0; j < arrangements[i].size(); j++) {
                result << arrangements[i][j];
                if (j < arrangements[i].size() - 1) result << ", ";
            }
            if (i < arrangements.size() - 1) result << '\n';
        }
        return result.str();
    }

    inline constexpr array<string_view, 5> dayNames = { "星期一", "星期二", "星期三", "星期四", "星期五" };
    inline constexpr array<string_view, 10> slotNames = { "│ 第1节  │", "│ 第2节  │", "│ 第3节  │", "│ 第4节  │", "│ 第5节  │", "│ 第6节  │", "│ 第7节  │", "│ 第8节  │", "│ 第9节  │", "│ 第10节 │" };

    [[nodiscard]] inline u64 getTextWidthWithPadding(const string_view& text) noexcept {
        u64 width = 0;
        for (u64 i = 0; i < text.length(); i++) {
            const u8 c = text[i];
            if ((c & 0x80) == 0) width++;
            else if ((c & 0xE0) == 0xC0) {
                width += 2;
                i++;
            }
            else if ((c & 0xF0) == 0xE0) {
                width += 2;
                i += 2;
            }
            else if ((c & 0xF8) == 0xF0) {
                width += 2;
                i += 3;
            }
        }
        return width;
    }

    inline void printSeparator(stringstream& result, const array<u64, 5>& maxWidths) noexcept {
        result << "├────────┼";
        for (u8 i = 0; i < 5; i++) {
            for (u64 j = 0; j < maxWidths[i] + 2; j++) result << "─";
            result << (i < 4 ? "┼" : "┤");
        }
        result << '\n';
    }

    inline void printBorder(stringstream& result, const array<u64, 5>& maxWidths, bool top) noexcept {
        result << (top ? "┌────────┬" : "└────────┴");
        for (u32 i = 0; i < 5; i++) {
            for (u64 j = 0; j < maxWidths[i] + 2; j++) result << "─";
            result << (i < 4 ? (top ? "┬" : "┴") : (top ? "┐" : "┘"));
        }
        if (top) result << '\n';
    }

    inline void printCell(stringstream& result, const string_view& text, u64 width) noexcept {
        const u64 textWidth = getTextWidthWithPadding(text), padding = width > textWidth ? width - textWidth : 0, leftPadding = padding / 2;
        for (u64 i = 0; i <= leftPadding; i++) result << " ";
        result << text;
        for (u64 i = 0; i <= padding - leftPadding; i++) result << " ";
        result << "│";
    }

    inline string printSchedules(const vector<Schedule>& schedules) noexcept {
        stringstream result;
        for (u64 i = 0; i < schedules.size(); i++) {
            array<u64, 5> maxWidths = { 0, 0, 0, 0, 0 };
            for (u8 j = 0; j < 5; j++) {
                maxWidths[j] = max(8ull, getTextWidthWithPadding(dayNames[j]));
                for (u8 k = 0; k < 10; k++) {
                    const string& courseName = schedules[i].slots[j][k];
                    if (!courseName.empty()) {
                        u64 len = getTextWidthWithPadding(courseName);
                        maxWidths[j] = max(maxWidths[j], len);
                    }
                }
            }
            printBorder(result, maxWidths, true);
            result << "│  节次  │";
            for (u8 j = 0; j < 5; j++) printCell(result, dayNames[j], maxWidths[j]);
            result << '\n';
            printSeparator(result, maxWidths);
            for (u8 j = 0; j < 10; j++) {
                result << slotNames[j];
                for (u8 k = 0; k < 5; k++) printCell(result, schedules[i].slots[k][j], maxWidths[k]);
                result << '\n';
                if (j < 9) printSeparator(result, maxWidths);
            }
            printBorder(result, maxWidths, false);
            if (i < schedules.size() - 1) result << '\n';
        }
        return result.str();
    }
}