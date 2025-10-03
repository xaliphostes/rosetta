/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include <sol/sol.hpp>
#include <typeinfo>

namespace rosetta {

    template <typename T> inline void registerPointerType(sol::state& lua)
    {
        // Sol3 has good pointer support, but you can add explicit converters
        lua.new_usertype<T*>(std::string(typeid(T*).name()),
            // Dereference operator
            sol::meta_function::multiplication, [](T* ptr) -> T& { return *ptr; });
    }

} // namespace rosetta