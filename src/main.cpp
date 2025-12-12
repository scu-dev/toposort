#include <array>
#include <iostream>
#include <string>
#include <vector>
#define CLI11_ENABLE_EXTRA_VALIDATORS 1
#include <CLI/CLI.hpp>

#include "meta.hpp"
#include "print.hpp"
#include "toposort.hpp"
#include "utils.hpp"

int main(int argc, char** argv) {
    typedef uint32_t u32;
    using std::array, std::string, std::ofstream, std::vector, std::cout, std::cerr, std::endl, CLI::App, CLI::CallForHelp, CLI::CallForVersion, CLI::ParseError, Toposort::Utils::getLastError, Toposort::Course, Toposort::Schedule, Toposort::printArrangements, Toposort::printSchedules;

    #if _TOPOSORT_WINDOWS
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);
    #endif
    App app;
    app.name(Toposort::TOPOSORT_APP_NAME);
    app.allow_windows_style_options(false);
    app.allow_config_extras(false);
    app.set_help_all_flag("");
    app.set_help_flag("-h,--help", "输出这条帮助信息并退出。");
    app.set_version_flag("-v, --version", Toposort::TOPOSORT_SEMATIC_VERSION, "显示版本信息并退出。");
    app.set_config("");
    app.footer(Toposort::TOPOSORT_COPYRIGHT_NOTICE);
    string inputFile;
    app.add_option("-i,--input", inputFile, "指定输入文件路径。")->required()->check(CLI::ExistingFile | CLI::ReadPermissions);
    string outputFile;
    app.add_option("-o,--output", outputFile, "指定输出文件路径。")->required();
    try { app.parse(argc, argv); }
    catch (const CallForHelp& e) {
        cout << app.help("", CLI::AppFormatMode::All) << endl;
        exit(0);
    }
    catch (const CallForVersion& e) {
        cout << Toposort::TOPOSORT_SEMATIC_VERSION << endl;
        exit(0);
    }
    catch (const ParseError& e) {
        cerr << "参数错误：(" << e.get_exit_code() << ")" << e.get_name() << " " << e.what() << endl;
        exit(1);
    }
    vector<Course> courses;
    vector<u32> semesterLimits;
    if (!Toposort::loadInfoFromFile(inputFile, courses, semesterLimits)) {
        cerr << "错误：" << getLastError() << endl;
        exit(1);
    }
    const vector<vector<string>> arrangements = Toposort::sortCourses(courses, semesterLimits);
    if (arrangements.empty()) {
        cerr << "错误：" << getLastError() << endl;
        exit(1);
    }
    string arrangementStr = printArrangements(arrangements);
    cout << arrangementStr << '\n';
    const vector<Schedule> schedules = Toposort::getSchedules(courses, arrangements);
    if (schedules.empty()) {
        cerr << "错误：" << getLastError() << endl;
        exit(1);
    }
    string scheduleStr = printSchedules(schedules);
    cout << scheduleStr << '\n';
    ofstream output(outputFile, std::ios::out);
    if (!output.is_open()) {
        cerr << "错误：无法打开输出文件：" << outputFile << endl;
        exit(1);
    }
    output << arrangementStr << '\n' << scheduleStr;
    cout << "已写入到输出文件：" << outputFile << endl;
    return 0;
}