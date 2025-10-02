/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 * 
 */
#include <iostream>
#include <sstream>

namespace rosetta {

    // Implementation of Introspectable methods (after TypeInfo is fully defined)
    inline Arg Introspectable::getMemberValue(const std::string& member_name) const
    {
        const auto& type_info = getTypeInfo();
        const auto* member = type_info.getMember(member_name);
        if (member) {
            return member->getter(this);
        }
        throw std::runtime_error("Member '" + member_name + "' not found");
    }

    inline void Introspectable::setMemberValue(const std::string& member_name, const Arg& value)
    {
        const auto& type_info = getTypeInfo();
        const auto* member = type_info.getMember(member_name);
        if (member) {
            member->setter(const_cast<void*>(static_cast<const void*>(this)), value);
        } else {
            throw std::runtime_error("Member '" + member_name + "' not found");
        }
    }

    inline Arg Introspectable::callMethod(const std::string& method_name, const Args& args)
    {
        const auto& type_info = getTypeInfo();
        const auto* method = type_info.getMethod(method_name);
        if (method) {
            return method->invoker(const_cast<void*>(static_cast<const void*>(this)), args);
        }
        throw std::runtime_error("Method '" + method_name + "' not found");
    }

    inline std::vector<std::string> Introspectable::getMemberNames() const
    {
        return getTypeInfo().getMemberNames();
    }

    inline std::vector<std::string> Introspectable::getMethodNames() const
    {
        return getTypeInfo().getMethodNames();
    }

    inline std::string Introspectable::getClassName() const { return getTypeInfo().class_name; }

    inline bool Introspectable::hasMember(const std::string& name) const
    {
        return getTypeInfo().getMember(name) != nullptr;
    }

    inline bool Introspectable::hasMethod(const std::string& name) const
    {
        return getTypeInfo().getMethod(name) != nullptr;
    }

    inline void Introspectable::printMemberValue(const std::string& member_name) const
    {
        const auto& type_info = getTypeInfo();
        const auto* member = type_info.getMember(member_name);
        if (member) {
            auto value = member->getter(this);
            std::cout << member_name << " (" << member->type_name << "): ";

            // Print based on type
            // TODO: refactoring for genericity!
            if (member->type_name == "string") {
                std::cout << std::any_cast<std::string>(value);
            } else if (member->type_name == "int") {
                std::cout << std::any_cast<int>(value);
            } else if (member->type_name == "double") {
                std::cout << std::any_cast<double>(value);
            } else if (member->type_name == "float") {
                std::cout << std::any_cast<float>(value);
            } else if (member->type_name == "bool") {
                std::cout << (std::any_cast<bool>(value) ? "true" : "false");
            } else {
                std::cout << "[" << member->type_name << " value]";
            }
            std::cout << std::endl;
        } else {
            std::cout << "Member '" << member_name << "' not found" << std::endl;
        }
    }

    inline void Introspectable::printClassInfo() const
    {
        const auto& type_info = getTypeInfo();
        std::cout << "Class: " << type_info.class_name << std::endl;

        std::cout << "Members:" << std::endl;
        for (const auto& member_name : type_info.getMemberNames()) {
            const auto* member = type_info.getMember(member_name);
            std::cout << "  " << member->type_name << " " << member_name << std::endl;
        }

        std::cout << "Methods:" << std::endl;
        for (const auto& method_name : type_info.getMethodNames()) {
            const auto* method = type_info.getMethod(method_name);
            std::cout << "  " << method->return_type << "  " << method_name;
            if (!method->parameter_types.empty()) {
                std::cout << "(";
                for (size_t i = 0; i < method->parameter_types.size(); ++i) {
                    if (i > 0) {
                        std::cout << ", ";
                    }
                    std::cout << method->parameter_types[i];
                }
                std::cout << ")";
            } else {
                std::cout << "()";
            }
            std::cout << std::endl;
        }
    }

    inline std::string Introspectable::toJSON() const
    {
        std::stringstream json;
        const auto& type_info = this->getTypeInfo();

        json << "{\n";
        json << "  \"className\": \"" << type_info.class_name << "\",\n";
        json << "  \"members\": [\n";

        auto member_names = type_info.getMemberNames();
        for (size_t i = 0; i < member_names.size(); ++i) {
            const auto* member = type_info.getMember(member_names[i]);
            json << "    {\n";
            json << "      \"name\": \"" << member->name << "\",\n";
            json << "      \"type\": \"" << member->type_name << "\",\n";
            json << "      \"value\": ";

            // Serialize current value
            auto value = member->getter(this);
            if (member->type_name == "string") {
                json << "\"" << std::any_cast<std::string>(value) << "\"";
            } else if (member->type_name == "int") {
                json << std::any_cast<int>(value);
            } else if (member->type_name == "double") {
                json << std::any_cast<double>(value);
            } else if (member->type_name == "bool") {
                json << (std::any_cast<bool>(value) ? "true" : "false");
            } else {
                json << "null";
            }

            json << "\n    }";
            if (i < member_names.size() - 1)
                json << ",";
            json << "\n";
        }

        json << "  ],\n";
        json << "  \"methods\": [\n";

        auto method_names = type_info.getMethodNames();
        for (size_t i = 0; i < method_names.size(); ++i) {
            const auto* method = type_info.getMethod(method_names[i]);
            json << "    {\n";
            json << "      \"name\": \"" << method->name << "\",\n";
            json << "      \"returnType\": \"" << method->return_type << "\",\n";
            json << "      \"parameters\": [";

            for (size_t j = 0; j < method->parameter_types.size(); ++j) {
                json << "\"" << method->parameter_types[j] << "\"";
                if (j < method->parameter_types.size() - 1)
                    json << ", ";
            }

            json << "]\n    }";
            if (i < method_names.size() - 1)
                json << ",";
            json << "\n";
        }

        json << "  ]\n";
        json << "}";

        return json.str();
    }

} // namespace rosetta