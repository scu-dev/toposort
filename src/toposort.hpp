#pragma once
#include <array>
#include <filesystem>
#include <fstream>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "utils.hpp"

namespace Toposort {
    typedef uint32_t u32;
    typedef uint64_t u64;
    using std::array, std::pair, std::string, std::vector, std::ifstream, std::filesystem::path, std::istringstream, std::stoul, std::move, std::to_string, std::queue, std::unordered_map, std::unordered_set, std::getline, Utils::normalize, Utils::setError;

    struct Course {
        string name, code;
        vector<string> prerequisites;
        u32 credit, semester;
    };

    [[nodiscard]] inline bool loadInfoFromFile(const string& filePathStr, vector<Course>& courses, vector<u32>& semesterLimits) noexcept {
        path filePath(filePathStr);
        if (!normalize(filePath)) {
            setError("无法规范化文件路径：" + filePathStr);
            return false;
        }
        ifstream file(filePath, std::ios::in);
        if (!file.is_open()) {
            setError("无法打开文件：" + filePath.string());
            return false;
        }
        u32 totalCourses = 0;
        string temp1;
        {
            if (!getline(file, temp1)) {
                setError("无法读取文件第一行。");
                return false;
            }
            istringstream iss(temp1);
            u32 count;
            while (iss >> count) {
                semesterLimits.push_back(count);
                totalCourses += count;
            }
            if (semesterLimits.empty()) {
                setError("第一行没有有效的学期课程数。");
                return false;
            }
            if (semesterLimits.size() & 1) {
                setError("学期数必须为偶数。");
                return false;
            }
        }
        string name, code, temp2, temp3;
        u32 credit, semester;
        vector<string> prereqs;
        while (getline(file, temp1)) {
            if (temp1.empty()) continue;
            istringstream iss(temp1);
            if (!getline(iss, code, ',')) continue;
            if (!getline(iss, name, ',')) continue;
            //学时
            if (!getline(iss, temp2, ',')) continue;
            try { credit = stoul(temp2); } catch (...) {
                setError("课程 " + code + "（" + name + "）的学时无效。");
                return false;
            }
            //指定学期
            if (!getline(iss, temp2, ',')) continue;
            try { semester = stoul(temp2); } catch (...) {
                setError("课程 " + code + "（" + name + "）的指定学期无效。");
                return false;
            }
            //先修课程（以分号分隔）
            if (getline(iss, temp2)) {
                istringstream prereqStream(temp2);
                while (getline(prereqStream, temp3, ';')) {
                    if (!temp3.empty()) prereqs.push_back(temp3);
                    else {
                        setError("课程 " + code + "（" + name + "）的先修课程列表格式错误。");
                        return false;
                    }
                }
            }
            courses.emplace_back(name, code, move(prereqs), credit, semester);
            prereqs.clear();
        }
        if (courses.size() != totalCourses) {
            setError("课程数量与第一行指定的总课程数不符。");
            return false;
        }
        return true;
    }

    [[nodiscard]] inline vector<vector<string>> sortCourses(const vector<Course>& courses, const vector<u32>& semesterLimits) noexcept {
        vector<vector<string>> result;
        if (courses.empty() || semesterLimits.empty()) {
            setError("数据无效。");
            return result;
        }
        unordered_map<string, u32> courseIndex;
        unordered_map<string, vector<u32>> adjacencyList;
        vector<u32> inDegree(courses.size(), 0);
        for (u32 i = 0; i < courses.size(); i++) courseIndex[courses[i].code] = i;
        for (u32 i = 0; i < courses.size(); i++) {
            for (const auto& prereq : courses[i].prerequisites) {
                auto it = courseIndex.find(prereq);
                if (it != courseIndex.end()) {
                    adjacencyList[prereq].push_back(i);
                    inDegree[i]++;
                }
            }
        }
        vector<bool> scheduled(courses.size(), false);
        u32 totalScheduled = 0;
        for (u64 currentSemester = 0; currentSemester < semesterLimits.size(); currentSemester++) {
            vector<string> arrangement;
            u32 limit = semesterLimits[currentSemester], scheduledCount = 0, totalCredits = 0;
            vector<u32> requiredCourses, availableCourses;
            for (u64 i = 0; i < courses.size(); i++) {
                if (scheduled[i] || inDegree[i] > 0) continue;
                if (courses[i].semester == currentSemester + 1) requiredCourses.push_back(i);
                else if (courses[i].semester == 0) availableCourses.push_back(i);
            }
            if (requiredCourses.size() > limit) {
                setError("第 " + to_string(currentSemester + 1) + " 学期的必修课程数量（" + to_string(requiredCourses.size()) + "）超过了学期限制（" + to_string(limit) + "）。");
                result.clear();
                return result;
            }
            u32 requiredCredits = 0;
            for (u32 idx : requiredCourses) requiredCredits += courses[idx].credit;
            if (requiredCredits > 50) {
                setError("第 " + to_string(currentSemester + 1) + " 学期的必修课程学分（" + to_string(requiredCredits) + "）超过了50学分的限制。");
                result.clear();
                return result;
            }
            for (u32 idx : requiredCourses) {
                if (scheduledCount >= 50) {
                    setError("单个学期课程数量超过了50门的限制。");
                    result.clear();
                    return result;
                }
                arrangement.push_back(courses[idx].code);
                scheduled[idx] = true;
                scheduledCount++;
                totalScheduled++;
                totalCredits += courses[idx].credit;
                for (u32 dependent : adjacencyList[courses[idx].code]) inDegree[dependent]--;
            }
            for (u32 idx : availableCourses) {
                if (scheduledCount >= limit || scheduledCount >= 50) break;
                if (totalCredits + courses[idx].credit > 50) continue;
                arrangement.push_back(courses[idx].code);
                scheduled[idx] = true;
                scheduledCount++;
                totalScheduled++;
                totalCredits += courses[idx].credit;
                for (u32 dependent : adjacencyList[courses[idx].code]) inDegree[dependent]--;
            }
            if (scheduledCount > 0) result.push_back(arrangement);
        }
        for (u64 i = 0; i < courses.size(); i++) if (!scheduled[i]) {
            if (courses[i].semester != 0) setError("课程 " + courses[i].code + "（" + courses[i].name + "）要求在第 " + to_string(courses[i].semester) + " 学期修读，但由于先修课程的限制无法满足。");
            else setError("课程 " + courses[i].code + "（" + courses[i].name + "）由于学期限制或先修课程的限制无法安排。");
            result.clear();
            return result;
        }
        return result;
    }

    struct Schedule {
        array<array<string, 10>, 5> slots;
    };

    struct CourseSlot {
        string code, name;
        u32 credits, sessionsPerWeek;
        vector<u32> sessionLengths;
    };

    [[nodiscard]] inline vector<Schedule> getSchedules(const vector<Course>& courses, const vector<vector<string>>& arrangements) noexcept {
        vector<Schedule> schedules;
        if (courses.empty() || arrangements.empty()) {
            setError("数据无效。");
            return schedules;
        }
        unordered_map<string, Course> courseMap;
        for (u64 i = 0; i < courses.size(); i++) courseMap[courses[i].code] = courses[i];
        for (u64 semesterIdx = 0; semesterIdx < arrangements.size(); semesterIdx++) {
            Schedule schedule;
            for (auto& day : schedule.slots) day.fill("");
            const vector<string>& arrangement = arrangements[semesterIdx];
            vector<CourseSlot> courseSlots;
            for (const string& courseCode : arrangement) {
                auto it = courseMap.find(courseCode);
                if (it == courseMap.end()) {
                    setError("无法找到课程代码 " + courseCode + " 对应的课程信息。");
                    schedules.clear();
                    return schedules;
                }
                const Course& course = it->second;
                CourseSlot slot;
                slot.code = course.code;
                slot.name = course.name;
                slot.credits = course.credit;
                if (course.credit <= 3) {
                    slot.sessionsPerWeek = 1;
                    slot.sessionLengths.push_back(course.credit);
                }
                else if (course.credit <= 6) {
                    u32 half = course.credit / 2;
                    slot.sessionsPerWeek = 2;
                    slot.sessionLengths.push_back(half);
                    slot.sessionLengths.push_back(course.credit - half);
                }
                else {
                    u32 third = course.credit / 3;
                    slot.sessionsPerWeek = 3;
                    slot.sessionLengths.push_back(third);
                    slot.sessionLengths.push_back(third);
                    slot.sessionLengths.push_back(course.credit - 2 * third);
                }
                courseSlots.push_back(slot);
            }
            vector<bool> slotUsed(50, false);
            auto canPlaceInSlots = [&](u32 day, u32 startSlot, u32 length) -> bool {
                if (startSlot + length > 10) return false;
                for (u32 i = 0; i < length; i++) if (slotUsed[day * 10 + startSlot + i]) return false;
                return true;
            };
            auto placeInSlots = [&](u32 day, u32 startSlot, u32 length, const string& name) {
                for (u32 i = 0; i < length; i++) {
                    schedule.slots[day][startSlot + i] = name;
                    slotUsed[day * 10 + startSlot + i] = true;
                }
            };
            auto findBestSlot = [&](u32 length, const vector<u32>& usedDays) -> pair<u32, u32> {
                vector<pair<u32, u32>> candidates;
                for (u32 day = 0; day < 5; day++) {
                    bool dayUsed = false;
                    for (u32 used : usedDays) if (used == day || (used > 0 && used - 1 == day) || (used < 4 && used + 1 == day)) {
                        dayUsed = true;
                        break;
                    }
                    if (dayUsed && usedDays.size() > 0) continue;
                    if (length == 2 && canPlaceInSlots(day, 0, 2)) candidates.push_back({day, 0});
                    if (length == 3 && canPlaceInSlots(day, 2, 3)) candidates.push_back({day, 2});
                    if (length == 2 && canPlaceInSlots(day, 5, 2)) candidates.push_back({day, 5});
                    if (length == 3 && canPlaceInSlots(day, 7, 3)) candidates.push_back({day, 7});
                    if (length == 1) {
                        if (canPlaceInSlots(day, 2, 1)) candidates.push_back({day, 2});
                        if (canPlaceInSlots(day, 3, 1)) candidates.push_back({day, 3});
                        if (canPlaceInSlots(day, 4, 1)) candidates.push_back({day, 4});
                        if (canPlaceInSlots(day, 7, 1)) candidates.push_back({day, 7});
                        if (canPlaceInSlots(day, 8, 1)) candidates.push_back({day, 8});
                        if (canPlaceInSlots(day, 9, 1)) candidates.push_back({day, 9});
                    }
                    if (candidates.empty()) {
                        if (length == 2 && canPlaceInSlots(day, 3, 2)) candidates.push_back({day, 3});
                        if (length == 2 && canPlaceInSlots(day, 7, 2)) candidates.push_back({day, 7});
                        if (length == 3 && canPlaceInSlots(day, 0, 3)) candidates.push_back({day, 0});
                        if (length == 3 && canPlaceInSlots(day, 5, 3)) candidates.push_back({day, 5});
                    }
                }
                if (candidates.empty()) {
                    for (u32 day = 0; day < 5; day++) for (u32 slot = 0; slot <= 10 - length; slot++) if (canPlaceInSlots(day, slot, length)) return {day, slot};
                    return {5, 0};
                }
                pair<u32, u32> best = candidates[0];
                for (const auto& candidate : candidates) if ((candidate.second != 0 && candidate.second != 7) && (best.second == 0 || best.second == 7)) best = candidate;
                return best;
            };
            for (const auto& courseSlot : courseSlots) {
                vector<u32> usedDays;
                for (u32 session = 0; session < courseSlot.sessionsPerWeek; session++) {
                    u32 length = courseSlot.sessionLengths[session];
                    auto [day, startSlot] = findBestSlot(length, usedDays);
                    if (day >= 5) {
                        setError("无法为课程 " + courseSlot.code + " 安排足够的课时。");
                        schedules.clear();
                        return schedules;
                    }
                    placeInSlots(day, startSlot, length, courseSlot.name);
                    usedDays.push_back(day);
                }
            }
            schedules.push_back(schedule);
        }
        return schedules;
    }
}