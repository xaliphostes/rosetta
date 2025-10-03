/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 * 
 */
namespace rosetta {

    template <typename T>
    inline LuaGenerator& LuaGenerator::bind_class(const std::string& class_name)
    {
        static_assert(
            std::is_base_of_v<Introspectable, T>, "Type must inherit from Introspectable");

        // Get type info from introspection system
        const auto& type_info = T::getStaticTypeInfo();
        std::string final_class_name = class_name.empty() ? type_info.class_name : class_name;

        // Prevent duplicate bindings
        if (bound_classes.find(final_class_name) != bound_classes.end()) {
            throw std::runtime_error("Class '" + final_class_name + "' already bound");
        }
        bound_classes.insert(final_class_name);

        // Create Sol3 usertype
        auto user_type = lua.new_usertype<T>(final_class_name);

        // Bind constructors
        bind_constructors<T>(user_type, type_info);

        // Bind all members as properties
        bind_members<T>(user_type, type_info);

        // Bind all methods
        bind_methods<T>(user_type, type_info);

        // Add introspection utilities
        bind_introspection_utilities<T>(user_type);

        return *this;
    }

    template <typename... Classes> inline LuaGenerator& LuaGenerator::bind_classes()
    {
        (bind_class<Classes>(), ...);
        return *this;
    }

    template <typename T>
    inline void LuaGenerator::bind_constructors(
        sol::usertype<T>& user_type, const TypeInfo& type_info)
    {
        const auto& constructors = type_info.getConstructors();

        if (constructors.empty()) {
            // Default constructor only
            user_type[sol::call_constructor] = sol::constructors<T()>();
            return;
        }

        // Register all constructors generically
        // Sol3 handles overload resolution automatically
        user_type[sol::call_constructor] = [constructors](sol::variadic_args va) -> T* {
            size_t arg_count = va.size();

            // Find matching constructor
            for (const auto& ctor : constructors) {
                if (ctor->parameter_types.size() == arg_count) {
                    // Convert Lua arguments to C++
                    std::vector<std::any> cpp_args;
                    for (size_t i = 0; i < arg_count; ++i) {
                        sol::object arg = va[i];
                        std::any cpp_arg;

                        const auto& param_type = ctor->parameter_types[i];
                        if (param_type == "string") {
                            cpp_arg = arg.as<std::string>();
                        } else if (param_type == "int") {
                            cpp_arg = arg.as<int>();
                        } else if (param_type == "double") {
                            cpp_arg = arg.as<double>();
                        } else if (param_type == "float") {
                            cpp_arg = arg.as<float>();
                        } else if (param_type == "bool") {
                            cpp_arg = arg.as<bool>();
                        }
                        cpp_args.push_back(cpp_arg);
                    }

                    // Create object using factory
                    void* raw_ptr = ctor->factory(cpp_args);
                    return static_cast<T*>(raw_ptr);
                }
            }

            throw std::runtime_error(
                "No matching constructor found for " + std::to_string(arg_count) + " arguments");
        };
    }

    template <typename T>
    inline void LuaGenerator::bind_members(sol::usertype<T>& user_type, const TypeInfo& type_info)
    {
        for (const auto& member_name : type_info.getMemberNames()) {
            const auto* member = type_info.getMember(member_name);
            if (!member)
                continue;

            // Create property with getter and setter
            user_type[member_name] = sol::property(
                // Getter
                [member_name](const T& obj) -> sol::object {
                    auto value = obj.getMemberValue(member_name);
                    const auto* mem = obj.getTypeInfo().getMember(member_name);

                    // Convert based on type
                    if (mem->type_name == "string") {
                        return sol::make_object(
                            obj.getTypeInfo().class_name, std::any_cast<std::string>(value));
                    } else if (mem->type_name == "int") {
                        return sol::make_object(
                            obj.getTypeInfo().class_name, std::any_cast<int>(value));
                    } else if (mem->type_name == "double") {
                        return sol::make_object(
                            obj.getTypeInfo().class_name, std::any_cast<double>(value));
                    } else if (mem->type_name == "float") {
                        return sol::make_object(
                            obj.getTypeInfo().class_name, std::any_cast<float>(value));
                    } else if (mem->type_name == "bool") {
                        return sol::make_object(
                            obj.getTypeInfo().class_name, std::any_cast<bool>(value));
                    }
                    return sol::lua_nil;
                },
                // Setter
                [member_name](T& obj, sol::object lua_value) {
                    const auto* mem = obj.getTypeInfo().getMember(member_name);
                    std::any cpp_value;

                    if (mem->type_name == "string") {
                        cpp_value = lua_value.as<std::string>();
                    } else if (mem->type_name == "int") {
                        cpp_value = lua_value.as<int>();
                    } else if (mem->type_name == "double") {
                        cpp_value = lua_value.as<double>();
                    } else if (mem->type_name == "float") {
                        cpp_value = lua_value.as<float>();
                    } else if (mem->type_name == "bool") {
                        cpp_value = lua_value.as<bool>();
                    }

                    obj.setMemberValue(member_name, cpp_value);
                });
        }
    }

    template <typename T>
    inline void LuaGenerator::bind_methods(sol::usertype<T>& user_type, const TypeInfo& type_info)
    {
        for (const auto& method_name : type_info.getMethodNames()) {
            const auto* method = type_info.getMethod(method_name);
            if (!method)
                continue;

            // Skip getter/setter methods that are already bound as properties
            if (is_getter_setter_method(method_name, type_info))
                continue;

            // Create Lua function wrapper
            user_type[method_name] = [method_name](T& obj, sol::variadic_args va) -> sol::object {
                const auto* method_info = obj.getTypeInfo().getMethod(method_name);

                if (va.size() != method_info->parameter_types.size()) {
                    throw std::runtime_error("Method '" + method_name + "' expects "
                        + std::to_string(method_info->parameter_types.size()) + " arguments, got "
                        + std::to_string(va.size()));
                }

                // Convert arguments
                std::vector<std::any> cpp_args;
                for (size_t i = 0; i < va.size(); ++i) {
                    sol::object arg = va[i];
                    const auto& param_type = method_info->parameter_types[i];

                    if (param_type == "string") {
                        cpp_args.push_back(arg.as<std::string>());
                    } else if (param_type == "int") {
                        cpp_args.push_back(arg.as<int>());
                    } else if (param_type == "double") {
                        cpp_args.push_back(arg.as<double>());
                    } else if (param_type == "float") {
                        cpp_args.push_back(arg.as<float>());
                    } else if (param_type == "bool") {
                        cpp_args.push_back(arg.as<bool>());
                    }
                }

                // Call method
                auto result = obj.callMethod(method_name, cpp_args);

                // Convert result back to Lua
                if (!result.has_value() || method_info->return_type == "void") {
                    return sol::lua_nil;
                }

                auto& type_info = obj.getTypeInfo();
                if (method_info->return_type == "string") {
                    return sol::make_object(
                        type_info.class_name, std::any_cast<std::string>(result));
                } else if (method_info->return_type == "int") {
                    return sol::make_object(type_info.class_name, std::any_cast<int>(result));
                } else if (method_info->return_type == "double") {
                    return sol::make_object(type_info.class_name, std::any_cast<double>(result));
                } else if (method_info->return_type == "float") {
                    return sol::make_object(type_info.class_name, std::any_cast<float>(result));
                } else if (method_info->return_type == "bool") {
                    return sol::make_object(type_info.class_name, std::any_cast<bool>(result));
                }

                return sol::lua_nil;
            };
        }
    }

    template <typename T>
    inline void LuaGenerator::bind_introspection_utilities(sol::usertype<T>& user_type)
    {
        user_type["getClassName"] = &T::getClassName;
        user_type["getMemberNames"] = &T::getMemberNames;
        user_type["getMethodNames"] = &T::getMethodNames;
        user_type["hasMember"] = &T::hasMember;
        user_type["hasMethod"] = &T::hasMethod;
        user_type["toJSON"] = &T::toJSON;

        // Dynamic member access
        user_type["getMemberValue"] = [](const T& obj, const std::string& name) -> sol::object {
            auto value = obj.getMemberValue(name);
            const auto* member = obj.getTypeInfo().getMember(name);

            if (member->type_name == "string") {
                return sol::make_object(
                    obj.getTypeInfo().class_name, std::any_cast<std::string>(value));
            } else if (member->type_name == "int") {
                return sol::make_object(obj.getTypeInfo().class_name, std::any_cast<int>(value));
            } else if (member->type_name == "double") {
                return sol::make_object(obj.getTypeInfo().class_name, std::any_cast<double>(value));
            }
            return sol::lua_nil;
        };

        user_type["setMemberValue"] = [](T& obj, const std::string& name, sol::object value) {
            const auto* member = obj.getTypeInfo().getMember(name);
            if (!member) {
                throw std::runtime_error("Member not found: " + name);
            }

            std::any cpp_value;
            if (member->type_name == "string") {
                cpp_value = value.as<std::string>();
            } else if (member->type_name == "int") {
                cpp_value = value.as<int>();
            } else if (member->type_name == "double") {
                cpp_value = value.as<double>();
            }
            obj.setMemberValue(name, cpp_value);
        };

        user_type["callMethod"]
            = [](T& obj, const std::string& name, sol::table args) -> sol::object {
            const auto* method = obj.getTypeInfo().getMethod(name);
            if (!method) {
                throw std::runtime_error("Method not found: " + name);
            }

            std::vector<std::any> cpp_args;
            for (size_t i = 1; i <= args.size(); ++i) {
                sol::object arg = args[i];
                // Convert based on expected parameter type
                if (i - 1 < method->parameter_types.size()) {
                    const auto& param_type = method->parameter_types[i - 1];
                    if (param_type == "string") {
                        cpp_args.push_back(arg.as<std::string>());
                    } else if (param_type == "int") {
                        cpp_args.push_back(arg.as<int>());
                    } else if (param_type == "double") {
                        cpp_args.push_back(arg.as<double>());
                    }
                }
            }

            auto result = obj.callMethod(name, cpp_args);

            if (!result.has_value() || method->return_type == "void") {
                return sol::lua_nil;
            }

            // Convert result
            if (method->return_type == "string") {
                return sol::make_object(
                    obj.getTypeInfo().class_name, std::any_cast<std::string>(result));
            } else if (method->return_type == "int") {
                return sol::make_object(obj.getTypeInfo().class_name, std::any_cast<int>(result));
            }

            return sol::lua_nil;
        };
    }

    inline bool LuaGenerator::is_getter_setter_method(
        const std::string& method_name, const TypeInfo& type_info) const
    {
        if (method_name.starts_with("get") && method_name.length() > 3) {
            std::string potential_member = method_name.substr(3);
            potential_member[0] = std::tolower(potential_member[0]);
            if (type_info.getMember(potential_member) != nullptr) {
                return true;
            }
        }

        if (method_name.starts_with("set") && method_name.length() > 3) {
            std::string potential_member = method_name.substr(3);
            potential_member[0] = std::tolower(potential_member[0]);
            if (type_info.getMember(potential_member) != nullptr) {
                return true;
            }
        }

        return false;
    }

    inline LuaGenerator& LuaGenerator::add_utilities()
    {
        lua["getAllClasses"] = [this]() {
            sol::table classes = lua.create_table();
            int idx = 1;
            for (const auto& name : bound_classes) {
                classes[idx++] = name;
            }
            return classes;
        };

        return *this;
    }

} // namespace rosetta
