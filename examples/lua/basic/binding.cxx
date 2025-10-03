// examples/scripting/lua/binding.cxx
#include "../../classes_demo.h"
#include <iostream>
#include <rosetta/generators/lua.h>
#include <sol/sol.hpp>

int main()
{
    // Create Lua state
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package);

    std::cout << "=== Automatic Lua Bindings Demo ===" << std::endl;

    // Bind classes automatically - ONLY 1 LINE!
    rosetta::LuaGenerator generator(lua);
    generator.bind_classes<Person, Vehicle>().add_utilities();

    std::cout << "Classes bound to Lua successfully!" << std::endl;
    std::cout << "Running Lua test script..." << std::endl;

    // Execute test script
    try {
        lua.script_file("test.lua");
    } catch (const sol::error& e) {
        std::cerr << "Lua error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n=== Lua bindings demo completed ===" << std::endl;
    return 0;
}