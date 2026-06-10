// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

#include <experimental/meta>
#include <print>
#include <string_view>

struct Point {
    int    x;
    int    y;
    double z;

    double norm() const { return x * x + y * y + z * z; }
    void   translate(int dx, int dy) {
        x += dx;
        y += dy;
    }
    Point scaled(double factor) const {
        return {int(x * factor), int(y * factor), z * factor};
    }
};

int main() {
    constexpr auto ctx = std::meta::access_context::current();

    std::println("fields of Point:");
    template for (constexpr auto member :
                  std::define_static_array(std::meta::nonstatic_data_members_of(^^Point, ctx))) {
        std::println("  {} : {}", std::meta::identifier_of(member),
                     std::meta::display_string_of(std::meta::type_of(member)));
    }

    // ---------------------------------

    std::println("methods of Point:");
    template for (constexpr auto fn :
                  std::define_static_array(std::meta::members_of(^^Point, ctx))) {
        if constexpr (std::meta::is_function(fn) && !std::meta::is_constructor(fn) &&
                      !std::meta::is_destructor(fn) &&
                      !std::meta::is_special_member_function(fn) &&
                      !std::meta::is_static_member(fn)) {
            std::print("  {} {}(",
                       std::meta::display_string_of(std::meta::return_type_of(fn)),
                       std::meta::identifier_of(fn));

            bool first = true;
            template for (constexpr auto param :
                          std::define_static_array(std::meta::parameters_of(fn))) {
                if (!first) {
                    std::print(", ");
                }
                first = false;
                std::print("{}", std::meta::display_string_of(std::meta::type_of(param)));
            }
            std::println(")");
        }
    }

    // ---------------------------------

    constexpr std::meta::info info = ^^Point;
    typename[:info:] p             = {.x = 1, .y = 2, .z = 2.1};

    return 0;
}
