#pragma once

#ifndef NDEBUG

#include <cstdint>
#include <iomanip>
#include <iostream>
#if DEBUG_SHOW_SOURCE
#include <fstream>
#include <unordered_map>
#endif
#include <source_location>
#include <type_traits>
#include <typeinfo>
#include <sstream>
#include <memory>
#if defined(__unix__) && __has_include(<cxxabi.h>)
#include <cxxabi.h>
#endif

struct debug {
private:
    std::ostringstream oss;

    enum {
        silent = 0,
        print = 1,
        panic = 2,
        supress = 3,
    } state;

    char const *line;// 当前代码行内容
    std::source_location const &loc;// 源代码位置

    static void uni_quotes(std::ostream &oss, std::string_view sv, char quote) {
        oss << quote;
        for (char c: sv) {
            switch (c) {
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            case '\\': oss << "\\\\"; break;
            case '\0': oss << "\\0"; break;
            default:
                if ((c >= 0 && c < 0x20) || c == 0x7F) {
                    auto f = oss.flags();
                    oss << "\\x" << std::hex << std::setfill('0')
                        << std::setw(2) << static_cast<int>(c);
                    oss.flags(f);
                } else {
                    if (c == quote) {
                        oss << '\\';
                    }
                    oss << c;
                }
                break;
            }
        }
        oss << quote;
    }

    static std::string uni_demangle(char const *name) {
#if defined(__unix__) && __has_include(<cxxabi.h>)
        int status;
        char *p = abi::__cxa_demangle(name, 0, 0, &status);
        std::string s = p ? p : name;
        std::free(p);
#else
        std::string s = name;
#endif
        return s;
    }

    template <class T0>
    static void uni_format(std::ostream &oss, T0 &&t) {
        using T = std::decay_t<T0>; // 移除顶层cv符，数组、函数转指针
        // std::id_convertible_v<T,U>T是否能被隐式转换成U
        // 能被隐式转换成std::string_view同时不是const char* 的情况
        if constexpr (std::is_convertible_v<T, std::string_view> &&
                      !std::is_same_v<T, char const *>) {
            uni_quotes(oss, t, '"');
        // 为bool的情况
        } else if constexpr (std::is_same_v<T, bool>) {
            auto f = oss.flags();
            // 输出标志，true输出为“true”，false输出为“false”
            oss << std::boolalpha << t;
            oss.flags(f);
        // 单个字符，char在某些编译器上是无符号的
        } else if constexpr (std::is_same_v<T, char> ||
                             std::is_same_v<T, signed char>) {
            uni_quotes(oss, {reinterpret_cast<char const *>(&t), 1}, '\'');
        // utf-8，utf-16，utf-32
        } else if constexpr (std::is_same_v<T, char8_t> ||
                             std::is_same_v<T, char16_t> ||
                             std::is_same_v<T, char32_t>) {
            auto f = oss.flags();
            oss << "'\\"
                << " xu U"[sizeof(T)] << std::hex << std::setfill('0')
                << std::setw(sizeof(T) * 2) << std::uppercase
                << static_cast<std::uint32_t>(t) << "'";
            oss.flags(f);
        // 无符号int类型
        } else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
            auto f = oss.flags();
            oss << "0x" << std::hex << std::setfill('0')
                << std::setw(sizeof(T) * 2) << std::uppercase;
            if constexpr (sizeof(T) == 1) {
                oss << static_cast<unsigned int>(t);
            } else {
                oss << t;
            }
            oss.flags(f);
        // 浮点数类型
        } else if constexpr (std::is_floating_point_v<T>) {
            auto f = oss.flags();
            oss << std::fixed
                << std::setprecision(std::numeric_limits<T>::digits10) << t;
            oss.flags(f);
        // 如果 T0 类型的对象可以通过 << 操作符输出到 std::ostream，则执行
        // oss << std::forward<T0>(t)。
        } else if constexpr (requires(std::ostream &oss, T0 &&t) {
                                 oss << std::forward<T0>(t);
                             }) {
            oss << std::forward<T0>(t);
        // 如果 T0 类型的对象可以通过 std::to_string 转换为字符串，则执行
        // oss << std::to_string(std::forward<T0>(t))。
        } else if constexpr (requires(T0 &&t) {
                     std::to_string(std::forward<T0>(t));
                     }) {
            oss << std::to_string(std::forward<T0>(t));
        // 如果 T0 类型的对象可以通过 std::begin 和 std::end 遍历（即支持范围遍历），
        // 则将其视为一个容器，并格式化输出其内容。
        } else if constexpr (requires(T0 &&t) {
                     std::begin(std::forward<T0>(t)) !=
                         std::end(std::forward<T0>(t));
                     }) {
            oss << '{'; // 输出容器的起始标记 '{'
            bool add_comma = false; // 标记是否需要添加逗号分隔符
            for (auto &&i: t) { // 遍历容器中的每个元素
            if (add_comma)
                oss << ", "; // 如果不是第一个元素，添加逗号分隔符
            add_comma = true;
            // 递归调用 uni_format 格式化输出容器中的元素
            uni_format(oss, std::forward<decltype(i)>(i));
            }
            oss << '}'; // 输出容器的结束标记 '}'
        // 如果 T0 类型的对象是一个元组（std::tuple 或类似类型），
        // 则递归格式化输出元组中的每个元素。
        } else if constexpr (requires(T0 &&t) {
                     std::tuple_size<T>::value;
                     }) {
            oss << '{'; // 输出元组的起始标记 '{'
            bool add_comma = false; // 标记是否需要添加逗号分隔符
            // 使用 std::apply 遍历元组中的每个元素，并递归调用 uni_format 格式化输出
            std::apply(
            [&](auto &&...args) {
                (([&] {
                 if (add_comma)
                     oss << ", "; // 如果不是第一个元素，添加逗号分隔符
                 add_comma = true;
                 (uni_format)(oss, std::forward<decltype(args)>(args)); // 格式化输出元素
                 }()),
                 ...);
            },
            t);
            oss << '}'; // 输出元组的结束标记 '}'
        // 如果 T0 类型的对象是一个枚举类型，则输出其底层整数值。
        } else if constexpr (std::is_enum_v<T>) {
            uni_format(oss, static_cast<std::underlying_type_t<T>>(t));
        // 如果 T0 类型的对象是 std::type_info，则输出其解码后的类型名称。
        // typeid运算符返回type_info类型
        } else if constexpr (std::is_same_v<T, std::type_info>) {
            oss << uni_demangle(t.name());
        // 如果 T0 类型的对象有 repr() 方法，则调用该方法并格式化输出其结果。
        } else if constexpr (requires(T0 &&t) { std::forward<T0>(t).repr(); }) {
            uni_format(oss, std::forward<T0>(t).repr());
        // 如果 T0 类型的对象有 repr(std::ostream&) 方法，则调用该方法直接输出。
        } else if constexpr (requires(T0 &&t) {
                     std::forward<T0>(t).repr(oss);
                     }) {
            std::forward<T0>(t).repr(oss);
        // 如果 T0 类型的对象支持全局 repr() 函数，则调用该函数并格式化输出其结果。
        } else if constexpr (requires(T0 &&t) { repr(std::forward<T0>(t)); }) {
            uni_format(oss, repr(std::forward<T0>(t)));
        // 如果 T0 类型的对象支持全局 repr(std::ostream&, T) 函数，则调用该函数直接输出。
        } else if constexpr (requires(T0 &&t) {
                     repr(oss, std::forward<T0>(t));
                     }) {
            repr(oss, std::forward<T0>(t));
        // 如果 T0 类型的对象是一个可空指针（支持解引用和布尔转换），
        // 则输出解引用后的值或 "nil"（如果为空）。
        } else if constexpr (requires(T0 const &t) {
                     (*t);
                     (bool)t;
                     }) {
            if ((bool)t) {
            uni_format(oss, *t); // 解引用并格式化输出
            } else {
            oss << "nil"; // 输出 "nil" 表示空指针
            }
        // 如果 T0 类型的对象支持 std::visit（例如 std::variant），
        // 则访问其值并递归格式化输出。
        } else if constexpr (requires(T0 const &t) {
                     visit([](auto const &) {}, t);
                     }) {
            visit([&oss](auto const &t) { uni_format(oss, t); }, t);
        // 对于所有其他类型，输出其解码后的类型名称和内存地址。
        } else {
            oss << '[' << uni_demangle(typeid(t).name()) << " at "
            << reinterpret_cast<void const *>(std::addressof(t)) << ']';
        }
    }

    debug &add_location_marks() {
        char const *fn = loc.file_name();
        for (char const *fp = fn; *fp; ++fp) {
            if (*fp == '/') {
                fn = fp + 1;
            }
        }
        oss << fn << ':' << loc.line() << ':' << '\t';
        if (line) {
            oss << '[' << line << ']' << '\t';
        } else {
#if DEBUG_SHOW_SOURCE
            static thread_local std::unordered_map<std::string, std::string>
                fileCache;
            auto key = std::to_string(loc.line()) + loc.file_name();
            if (auto it = fileCache.find(key);
                it != fileCache.end() && !it->second.empty()) [[likely]] {
                oss << '[' << it->second << ']';
            } else if (auto file = std::ifstream(loc.file_name());
                       file.is_open()) [[likely]] {
                std::string line;
                for (int i = 0; i < loc.line(); ++i) {
                    if (!std::getline(file, line)) [[unlikely]] {
                        line.clear();
                        break;
                    }
                }
                if (auto pos = line.find_first_not_of(" \t\r\n");
                    pos != line.npos) [[likely]] {
                    line = line.substr(pos);
                }
                if (!line.empty()) [[likely]] {
                    if (line.back() == ';') [[likely]] {
                        line.pop_back();
                    }
                    oss << '[' << line << ']';
                }
                fileCache.try_emplace(key, std::move(line));
            } else {
                oss << '[' << '?' << ']';
                fileCache.try_emplace(key);
            }
#endif
        }
        oss << ' ';
        return *this;
    }

    template <class T>
    struct debug_condition {
    private:
        debug &d;
        T const &t;

        template <class U>
        debug &check(bool cond, U const &u, char const *sym) {
            if (!cond) [[unlikely]] {
                d.on_error("assertion failed:") << t << sym << u;
            }
            return d;
        }

    public:
        explicit debug_condition(debug &d, T const &t) noexcept : d(d), t(t) {}

        template <class U>
        debug &operator<(U const &u) {
            return check(t < u, u, "<");
        }

        template <class U>
        debug &operator>(U const &u) {
            return check(t > u, u, ">");
        }

        template <class U>
        debug &operator<=(U const &u) {
            return check(t <= u, u, "<=");
        }

        template <class U>
        debug &operator>=(U const &u) {
            return check(t >= u, u, ">=");
        }

        template <class U>
        debug &operator==(U const &u) {
            return check(t == u, u, "==");
        }

        template <class U>
        debug &operator!=(U const &u) {
            return check(t != u, u, "!=");
        }
    };

    debug &on_error(char const *msg) {
        if (state != supress) {
            state = panic;
            add_location_marks();
        } else {
            oss << ' ';
        }
        oss << msg;
        return *this;
    }

    template <class T>
    debug &on_print(T &&t) {
        if (state == supress)
            return *this;
        if (state == silent) {
            state = print;
            add_location_marks();
        } else {
            oss << ' ';
        }
        uni_format(oss, std::forward<T>(t));
        return *this;
    }

public:
    debug(bool enable = true, char const *line = nullptr,
          std::source_location const &loc =
              std::source_location::current()) noexcept
        : state(enable ? silent : supress),
          line(line),
          loc(loc) {}

    debug(debug &&) = delete;
    debug(debug const &) = delete;

    template <class T>
    debug_condition<T> check(T const &t) noexcept {
        return debug_condition<T>{*this, t};
    }

    template <class T>
    debug_condition<T> operator>>(T const &t) noexcept {
        return debug_condition<T>{*this, t};
    }

    debug &fail(bool fail = true) {
        if (fail) [[unlikely]] {
            on_error("error:");
        } else {
            state = supress;
        }
        return *this;
    }

    debug &on(bool enable) {
        if (!enable) [[likely]] {
            state = supress;
        }
        return *this;
    }

    template <class T>
    debug &operator<<(T &&t) {
        return on_print(std::forward<T>(t));
    }

    template <class T>
    debug &operator,(T &&t) {
        return on_print(std::forward<T>(t));
    }

    ~debug() noexcept(false) {
        if (state == panic) [[unlikely]] {
            throw std::runtime_error(oss.str());
        }
        if (state == print) {
            oss << '\n';
            std::cerr << oss.str();
        }
    }
};

#else

struct debug {
    debug(bool = true, char const * = nullptr) noexcept {}

    debug(debug &&) = delete;
    debug(debug const &) = delete;

    template <class T>
    debug &operator,(T &&) {
        return *this;
    }

    template <class T>
    debug &operator<<(T &&) {
        return *this;
    }

    debug &on(bool) {
        return *this;
    }

    debug &fail(bool = true) {
        return *this;
    }

    ~debug() noexcept(false) {}

private:
    struct debug_condition {
        debug &d;

        explicit debug_condition(debug &d) : d(d) {}

        template <class U>
        debug &operator<(U const &) {
            return d;
        }

        template <class U>
        debug &operator>(U const &) {
            return d;
        }

        template <class U>
        debug &operator<=(U const &) {
            return d;
        }

        template <class U>
        debug &operator>=(U const &) {
            return d;
        }

        template <class U>
        debug &operator==(U const &) {
            return d;
        }

        template <class U>
        debug &operator!=(U const &) {
            return d;
        }
    };

public:
    template <class T>
    debug_condition check(T const &) noexcept {
        return debug_condition{*this};
    }

    template <class T>
    debug_condition operator>>(T const &) noexcept {
        return debug_condition{*this};
    }
};

#endif
