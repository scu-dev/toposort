#pragma once
#include <filesystem>
#include <iostream>
#include <string>

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__NT__)
    #define _TOPOSORT_WINDOWS 1
    #include <windows.h> // IWYU pragma: keep
#else
    #define _TOPOSORT_UNIX 1
    #include <unistd.h> // IWYU pragma: keep
#endif

namespace Toposort::Utils {
    using std::string, std::filesystem::path, std::error_code, std::cout, std::endl;

    #define STR(p) reinterpret_cast<const char*>(p.u8string().c_str())

    inline string errorMessage;

    [[nodiscard]] inline string getLastError() noexcept { return errorMessage; }
    inline void setError(const string& err) noexcept { errorMessage = err; }

    inline bool normalize(path& p) noexcept {
        error_code ec;
        p = absolute(p, ec);
        if (ec) return false;
        p = p.lexically_normal();
        p.make_preferred();
        return true;
    }
}