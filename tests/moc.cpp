// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Prototype v8: Qt-moc-style signals / slots / properties built on top of
// C++26 reflection (P2996) + annotations (P3394). No external moc tool.
//
// What this exercises:
//   - [[= signal]]-tagged Signal<...> data members
//   - [[= slot]]-tagged methods
//   - [[= property{"name", "notifySig"}]]-tagged data members
//   - rosetta::moc::connect<"sig","slot">(sender, receiver) — checked at
//     compile time (typo'd name -> static_assert)
//   - rosetta::moc::get<"name">(obj) / set<"name">(obj, v) — set is
//     equality-gated and fires the NOTIFY signal on a real change
//
// Requires: -freflection -freflection-latest -fannotation-attributes

#include <print>
#include <rosetta/mini_moc.h>
#include <string>

// ----------------------------------------------------------

class Person {
public:
    [[= rosetta::moc::signal]]
    rosetta::moc::Signal<std::string const &> nameChanged;

    [[= rosetta::moc::signal]]
    rosetta::moc::Signal<int> ageChanged;

    [[= rosetta::moc::property{"name", "nameChanged"}]]
    std::string m_name;

    [[= rosetta::moc::property{"age", "ageChanged"}]]
    int m_age = 0;

    [[= rosetta::moc::property{"id"}]] // no NOTIFY
    int m_id = 0;
};

// ----------------------------------------------------------

struct Logger {
    int total = 0;

    [[= rosetta::moc::slot]]
    void onAge(int v) {
        total += v;
        std::println("  [slot] onAge({}) -> total={}", v, total);
    }

    [[= rosetta::moc::slot]]
    void onName(std::string const &s) {
        std::println("  [slot] onName({})", s);
    }
};

// ----------------------------------------------------------

int main() {
    Person p;
    Logger l;

    rosetta::moc::connect<"ageChanged", "onAge">(p, l);
    rosetta::moc::connect<"nameChanged", "onName">(p, l);

    std::println("-- set name --");
    rosetta::moc::set<"name">(p, std::string{"Ada"});

    std::println("-- set age 30 (fires) --");
    rosetta::moc::set<"age">(p, 30);

    std::println("-- set age 30 again (equality-gated, silent) --");
    rosetta::moc::set<"age">(p, 30);

    std::println("-- set id 42 (no NOTIFY declared) --");
    rosetta::moc::set<"id">(p, 42);

    std::println("\nfinal: name={}, age={}, id={}, logger.total={}",
                 rosetta::moc::get<"name">(p),
                 rosetta::moc::get<"age">(p),
                 rosetta::moc::get<"id">(p),
                 l.total);
    return 0;
}
