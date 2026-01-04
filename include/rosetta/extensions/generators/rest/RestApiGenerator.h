#pragma once
#include "../common/CodeWriter.h"
#include <rosetta/rosetta.h>
#include <set>

// ============================================================================
// REST API Generator
// Generates a C++ REST server exposing Rosetta-registered classes
// Uses cpp-httplib (header-only HTTP library)
// ============================================================================

class RestApiGenerator : public CodeWriter {
public:
    using CodeWriter::CodeWriter;

    void generate() override {
        // Detect abstract classes before generating
        detect_abstract_classes();

        write_header();
        write_includes();
        write_json_helpers();
        write_object_store();
        write_class_handlers();
        write_server_setup();
        write_main();
    }

private:
    using MethodMeta = rosetta::core::Registry::MetadataHolder::MethodMeta;

    // Track which classes are abstract (not instantiable via REST)
    std::set<std::string> abstract_classes_;

    // Detect abstract classes that cannot be instantiated via REST API
    void detect_abstract_classes() {
        auto &registry = rosetta::Registry::instance();

        for (const auto &name : registry.list_classes()) {
            auto *holder = registry.get_by_name(name);
            if (!holder || config_.should_skip_class(name))
                continue;

            if (is_abstract_class_impl(name, holder)) {
                abstract_classes_.insert(name);
            }
        }
    }

    // Implementation of abstract class detection with multiple heuristics
    bool is_abstract_class_impl(const std::string                             &name,
                                const rosetta::core::Registry::MetadataHolder *holder) const {
        // 1. Check if explicitly in skip_classes config
        for (const auto &skip : config_.skip_classes) {
            if (skip == name || skip == name + "*") {
                return true;
            }
        }

        // 2. Check if class has NO constructors at all
        auto ctors = holder->get_constructors();
        if (ctors.empty()) {
            auto methods = holder->get_methods();
            if (!methods.empty()) {
                // Has methods but no constructors - likely abstract
                return true;
            }
        }

        // 3. Check if ALL registered constructors require callback parameters
        if (!ctors.empty()) {
            bool has_rest_callable_ctor = false;
            for (const auto &ctor : ctors) {
                bool ctor_has_callback = false;
                for (const auto &pt : ctor.get_param_types()) {
                    if (is_function_type(pt)) {
                        ctor_has_callback = true;
                        break;
                    }
                }
                if (!ctor_has_callback) {
                    has_rest_callable_ctor = true;
                    break;
                }
            }

            if (!has_rest_callable_ctor) {
                return true;
            }
        }

        // 4. Check naming conventions for abstract base classes
        std::string simple_name = get_simple_class_name(name);

        // Classes starting with "Base" followed by uppercase
        if (simple_name.length() > 4 && simple_name.substr(0, 4) == "Base" &&
            std::isupper(simple_name[4])) {
            return true;
        }

        // 5. Check if this class is a base class for others with no constructors
        if (is_base_class_for_others(name, holder) && ctors.empty()) {
            return true;
        }

        return false;
    }

    // Check if a class is used as a base class for other registered classes
    bool is_base_class_for_others(const std::string                             &name,
                                  const rosetta::core::Registry::MetadataHolder *holder) const {
        auto &registry = rosetta::Registry::instance();

        for (const auto &other_name : registry.list_classes()) {
            if (other_name == name)
                continue;

            auto *other_holder = registry.get_by_name(other_name);
            if (!other_holder)
                continue;

            std::string base = other_holder->get_base_class();
            if (base.empty())
                continue;

            if (base == name)
                return true;

            std::string base_simple = get_simple_class_name(base);
            std::string name_simple = get_simple_class_name(name);
            if (base_simple == name_simple)
                return true;
        }

        return false;
    }

    std::string get_simple_class_name(const std::string &name) const {
        size_t pos = name.rfind("::");
        if (pos != std::string::npos) {
            return name.substr(pos + 2);
        }
        return name;
    }

    // Get the binding name for a class (respects strip_namespaces config)
    std::string get_binding_name(const std::string &name) const {
        return config_.binding_name(name);
    }

    // Convert a C++ type to human-readable form for API display
    std::string human_readable_type(const std::string &t) const {
        std::string r = t;
        size_t      pos;

        // Remove std::__1:: (libc++ internal namespace)
        while ((pos = r.find("std::__1::")) != std::string::npos) {
            r.replace(pos, 10, "std::");
        }

        // Remove allocator template arguments
        size_t alloc_start;
        while ((alloc_start = r.find(", std::allocator<")) != std::string::npos) {
            size_t depth     = 1;
            size_t alloc_end = alloc_start + 17;
            while (alloc_end < r.size() && depth > 0) {
                if (r[alloc_end] == '<')
                    depth++;
                else if (r[alloc_end] == '>')
                    depth--;
                alloc_end++;
            }
            r.erase(alloc_start, alloc_end - alloc_start);
        }

        // Remove char_traits template arguments
        while ((pos = r.find(", std::char_traits<char>")) != std::string::npos) {
            r.erase(pos, 24);
        }

        // Convert basic_string<char> to string
        while ((pos = r.find("std::basic_string<char>")) != std::string::npos) {
            r.replace(pos, 23, "string");
        }
        while ((pos = r.find("basic_string<char>")) != std::string::npos) {
            r.replace(pos, 18, "string");
        }

        // Simplify std::vector to vector
        while ((pos = r.find("std::vector")) != std::string::npos) {
            r.replace(pos, 11, "vector");
        }

        // Simplify std::string to string
        while ((pos = r.find("std::string")) != std::string::npos) {
            r.replace(pos, 11, "string");
        }

        // Simplify std::function to function
        while ((pos = r.find("std::function")) != std::string::npos) {
            r.replace(pos, 13, "function");
        }

        // Remove const and & for cleaner display
        while ((pos = r.find("const ")) != std::string::npos)
            r.erase(pos, 6);
        while ((pos = r.find(" const")) != std::string::npos)
            r.erase(pos, 6);
        while ((pos = r.find("&")) != std::string::npos)
            r.erase(pos, 1);

        // Trim whitespace
        while (!r.empty() && r[0] == ' ')
            r.erase(0, 1);
        while (!r.empty() && r.back() == ' ')
            r.pop_back();

        return r;
    }

    void write_header() {
        line("// ============================================================================");
        line("// AUTO-GENERATED REST API SERVER - DO NOT EDIT");
        line("// Generated by binding_generator from Rosetta introspection");
        line("// Module: " + config_.module_name);
        line("// ============================================================================");
        line();
    }

    void write_includes() {
        line("#include <httplib.h>");
        line("#include <nlohmann/json.hpp>");
        line();
        line("#include <memory>");
        line("#include <map>");
        line("#include <string>");
        line("#include <mutex>");
        line("#include <atomic>");
        line("#include <stdexcept>");
        line("#include <iostream>");
        line();
        line("#include <rosetta/rosetta.h>");
        line();

        // Include source headers from config
        if (!config_.source_headers.empty()) {
            line("// Project headers");
            for (const auto &header : config_.source_headers) {
                line("#include <" + header + ">");
            }
            line();
        }

        // Include registration header
        if (!config_.registration_header.empty()) {
            line(config_.get_registration_include());
            line();
        }

        line("using json = nlohmann::json;");
        line();
    }

    void write_json_helpers() {
        line("// ============================================================================");
        line("// JSON Conversion Helpers");
        line("// ============================================================================");
        line("namespace {");
        line();
        indent();

        // std::vector conversions
        line("// std::vector conversions");
        line("json vector_double_to_json(const std::vector<double>& vec) {");
        indent();
        line("return json(vec);");
        dedent();
        line("}");
        line();

        line("std::vector<double> json_to_vector_double(const json& j) {");
        indent();
        line("return j.get<std::vector<double>>();");
        dedent();
        line("}");
        line();

        line("json vector_int_to_json(const std::vector<int>& vec) {");
        indent();
        line("return json(vec);");
        dedent();
        line("}");
        line();

        line("std::vector<int> json_to_vector_int(const json& j) {");
        indent();
        line("return j.get<std::vector<int>>();");
        dedent();
        line("}");
        line();

        line("json vector_float_to_json(const std::vector<float>& vec) {");
        indent();
        line("return json(vec);");
        dedent();
        line("}");
        line();

        line("std::vector<float> json_to_vector_float(const json& j) {");
        indent();
        line("return j.get<std::vector<float>>();");
        dedent();
        line("}");
        line();

        // Generic value to JSON
        line("template<typename T>");
        line("json value_to_json(const T& val) {");
        indent();
        line("return json(val);");
        dedent();
        line("}");
        line();

        // Error response helper
        line("json error_response(const std::string& message, int code = 400) {");
        indent();
        line("return json{{\"error\", true}, {\"message\", message}, {\"code\", code}};");
        dedent();
        line("}");
        line();

        // Success response helper
        line("json success_response(const json& data = nullptr) {");
        indent();
        line("json resp = {{\"error\", false}};");
        line("if (!data.is_null()) resp[\"data\"] = data;");
        line("return resp;");
        dedent();
        line("}");
        line();

        dedent();
        line("} // anonymous namespace");
        line();
    }

    void write_object_store() {
        line("// ============================================================================");
        line("// Object Store - Manages instances created via REST API");
        line("// ============================================================================");
        line();
        line("class ObjectStore {");
        line("public:");
        indent();

        line("static ObjectStore& instance() {");
        indent();
        line("static ObjectStore store;");
        line("return store;");
        dedent();
        line("}");
        line();

        line("// Store an object and return its ID");
        line("template<typename T>");
        line("std::string store(std::shared_ptr<T> obj, const std::string& class_name) {");
        indent();
        line("std::lock_guard<std::mutex> lock(mutex_);");
        line("std::string id = class_name + \"_\" + std::to_string(next_id_++);");
        line("objects_[id] = std::static_pointer_cast<void>(obj);");
        line("class_names_[id] = class_name;");
        line("return id;");
        dedent();
        line("}");
        line();

        line("// Retrieve an object by ID");
        line("template<typename T>");
        line("std::shared_ptr<T> get(const std::string& id) {");
        indent();
        line("std::lock_guard<std::mutex> lock(mutex_);");
        line("auto it = objects_.find(id);");
        line("if (it == objects_.end()) return nullptr;");
        line("return std::static_pointer_cast<T>(it->second);");
        dedent();
        line("}");
        line();

        line("// Get raw void pointer for Rosetta invocation");
        line("void* get_void_ptr(const std::string& id) {");
        indent();
        line("std::lock_guard<std::mutex> lock(mutex_);");
        line("auto it = objects_.find(id);");
        line("if (it == objects_.end()) return nullptr;");
        line("return it->second.get();");
        dedent();
        line("}");
        line();

        line("// Check if object exists");
        line("bool exists(const std::string& id) {");
        indent();
        line("std::lock_guard<std::mutex> lock(mutex_);");
        line("return objects_.find(id) != objects_.end();");
        dedent();
        line("}");
        line();

        line("// Get class name for an object");
        line("std::string get_class(const std::string& id) {");
        indent();
        line("std::lock_guard<std::mutex> lock(mutex_);");
        line("auto it = class_names_.find(id);");
        line("return it != class_names_.end() ? it->second : \"\";");
        dedent();
        line("}");
        line();

        line("// Remove an object");
        line("bool remove(const std::string& id) {");
        indent();
        line("std::lock_guard<std::mutex> lock(mutex_);");
        line("objects_.erase(id);");
        line("return class_names_.erase(id) > 0;");
        dedent();
        line("}");
        line();

        line("// List all objects");
        line("json list_objects() {");
        indent();
        line("std::lock_guard<std::mutex> lock(mutex_);");
        line("json arr = json::array();");
        line("for (const auto& [id, cls] : class_names_) {");
        indent();
        line("arr.push_back({{\"id\", id}, {\"class\", cls}});");
        dedent();
        line("}");
        line("return arr;");
        dedent();
        line("}");
        line();

        dedent();
        line("private:");
        indent();
        line("std::map<std::string, std::shared_ptr<void>> objects_;");
        line("std::map<std::string, std::string> class_names_;");
        line("std::mutex mutex_;");
        line("std::atomic<uint64_t> next_id_{1};");
        dedent();
        line("};");
        line();
    }

    void write_class_handlers() {
        line("// ============================================================================");
        line("// Class-specific REST Handlers");
        line("// ============================================================================");
        line();

        auto &registry = rosetta::Registry::instance();

        for (const auto &name : registry.list_classes()) {
            auto *holder = registry.get_by_name(name);
            if (holder && !config_.should_skip_class(name)) {
                // Skip abstract classes
                if (abstract_classes_.count(name) > 0) {
                    line("// Skipping abstract class: " + name);
                    line();
                    continue;
                }
                write_class_handler(name, holder);
            }
        }
    }

    // Check if a class is abstract (uses pre-computed set)
    bool is_abstract_class(const std::string &name) const {
        return abstract_classes_.count(name) > 0;
    }

    void write_class_handler(const std::string                             &name,
                             const rosetta::core::Registry::MetadataHolder *holder) {
        std::string cpp_type     = holder->get_cpp_type_name();
        std::string binding_name = get_binding_name(name);
        std::string handler_name = binding_name + "Handler";

        line("// --- " + binding_name + " Handler ---");
        line("namespace " + handler_name + " {");
        line();
        indent();

        // Get class info - use binding_name for API, cpp_type for internal use
        line("json get_info() {");
        indent();
        line("json info;");
        line("info[\"name\"] = \"" + binding_name + "\";");
        line("info[\"cpp_type\"] = \"" + cpp_type + "\";");

        // Constructors
        line("info[\"constructors\"] = json::array();");
        for (const auto &ctor : holder->get_constructors()) {
            auto params   = ctor.get_param_types();
            bool has_func = false;
            for (const auto &p : params) {
                if (is_function_type(p)) {
                    has_func = true;
                    break;
                }
            }
            line("{");
            indent();
            line("json ctor_info;");
            line("ctor_info[\"params\"] = json::array();");
            for (const auto &p : params) {
                // Use human-readable type for API display
                line("ctor_info[\"params\"].push_back(\"" + human_readable_type(p) + "\");");
            }
            line("ctor_info[\"rest_callable\"] = " + std::string(has_func ? "false" : "true") +
                 ";");
            line("info[\"constructors\"].push_back(ctor_info);");
            dedent();
            line("}");
        }

        // Methods
        line("info[\"methods\"] = json::array();");
        for (const auto &m : holder->get_methods()) {
            if (!config_.should_skip_method(name, m)) {
                auto method_info    = holder->get_method_info(m);
                bool has_func_param = has_function_parameter(holder, m);
                line("{");
                indent();
                line("json method_info;");
                line("method_info[\"name\"] = \"" + m + "\";");
                // Use human-readable types for API display
                line("method_info[\"return_type\"] = \"" +
                     human_readable_type(method_info.get_return_type_str()) + "\";");
                line("method_info[\"params\"] = json::array();");
                for (const auto &p : method_info.get_param_types_str()) {
                    line("method_info[\"params\"].push_back(\"" + human_readable_type(p) + "\");");
                }
                line("method_info[\"is_const\"] = " +
                     std::string(method_info.is_const ? "true" : "false") + ";");
                line("method_info[\"is_lambda\"] = " +
                     std::string(method_info.is_lambda ? "true" : "false") + ";");
                line("method_info[\"rest_callable\"] = " +
                     std::string(has_func_param ? "false" : "true") + ";");
                line("info[\"methods\"].push_back(method_info);");
                dedent();
                line("}");
            }
        }

        line("return info;");
        dedent();
        line("}");
        line();

        // Create instance
        write_create_handler(name, cpp_type, binding_name, holder);

        // Call method
        write_method_handlers(name, cpp_type, binding_name, holder);

        dedent();
        line("} // namespace " + handler_name);
        line();
    }

    void write_create_handler(const std::string &name, const std::string &cpp_type,
                              const std::string                             &binding_name,
                              const rosetta::core::Registry::MetadataHolder *holder) {
        line("json create(const json& params) {");
        indent();
        line("try {");
        indent();
        line("std::shared_ptr<" + cpp_type + "> obj;");

        auto ctors = holder->get_constructors();

        // Filter constructors - skip those with std::function parameters
        std::vector<rosetta::core::Registry::MetadataHolder::ConstructorMeta> valid_ctors;
        for (const auto &ctor : ctors) {
            bool has_func = false;
            for (const auto &pt : ctor.get_param_types()) {
                if (is_function_type(pt)) {
                    has_func = true;
                    break;
                }
            }
            if (!has_func) {
                valid_ctors.push_back(ctor);
            }
        }

        if (valid_ctors.empty()) {
            if (ctors.empty()) {
                // No constructors at all - try default
                line("obj = std::make_shared<" + cpp_type + ">();");
            } else {
                // All constructors have function params
                line("return error_response(\"No REST-callable constructors available (all require "
                     "callback functions)\");");
            }
        } else {
            line("size_t argc = params.is_array() ? params.size() : 0;");
            line();

            bool first = true;
            for (const auto &ctor : valid_ctors) {
                auto        params_types = ctor.get_param_types();
                std::string cond         = first ? "if" : "} else if";
                first                    = false;

                line(cond + " (argc == " + std::to_string(params_types.size()) + ") {");
                indent();

                if (params_types.empty()) {
                    line("obj = std::make_shared<" + cpp_type + ">();");
                } else {
                    std::vector<std::string> args;
                    for (size_t i = 0; i < params_types.size(); ++i) {
                        args.push_back(generate_json_extraction(
                            params_types[i], "params[" + std::to_string(i) + "]"));
                    }
                    line("obj = std::make_shared<" + cpp_type + ">(" + join(args, ", ") + ");");
                }
                dedent();
            }
            line("} else {");
            indent();
            line("return error_response(\"Invalid number of constructor arguments\");");
            dedent();
            line("}");
        }

        line();
        // Use binding_name for object ID prefix
        line("std::string id = ObjectStore::instance().store(obj, \"" + binding_name + "\");");
        line("return success_response({{\"id\", id}, {\"class\", \"" + binding_name + "\"}});");

        dedent();
        line("} catch (const std::exception& e) {");
        indent();
        line("return error_response(e.what());");
        dedent();
        line("}");
        dedent();
        line("}");
        line();
    }

    void write_method_handlers(const std::string &name, const std::string &cpp_type,
                               const std::string                             &binding_name,
                               const rosetta::core::Registry::MetadataHolder *holder) {
        line("json call_method(const std::string& id, const std::string& method, const json& "
             "params) {");
        indent();
        line("auto obj = ObjectStore::instance().get<" + cpp_type + ">(id);");
        line("if (!obj) {");
        indent();
        line("return error_response(\"Object not found: \" + id, 404);");
        dedent();
        line("}");
        line();
        line("try {");
        indent();

        bool first = true;
        for (const auto &m : holder->get_methods()) {
            if (config_.should_skip_method(name, m))
                continue;

            // Skip methods with std::function parameters
            if (has_function_parameter(holder, m)) {
                continue;
            }

            auto info        = holder->get_method_info(m);
            auto param_types = info.get_param_types_str();
            auto return_type = info.get_return_type_str();

            std::string cond = first ? "if" : "} else if";
            first            = false;

            line(cond + " (method == \"" + m + "\") {");
            indent();

            // Check param count
            if (!param_types.empty()) {
                line("if (!params.is_array() || params.size() != " +
                     std::to_string(param_types.size()) + ") {");
                indent();
                line("return error_response(\"Expected " + std::to_string(param_types.size()) +
                     " parameters for " + m + "\");");
                dedent();
                line("}");
            }

            // For lambda methods, use Rosetta invocation
            if (info.is_lambda) {
                write_rosetta_method_invocation(cpp_type, m, param_types, return_type);
            } else {
                // Direct method call for regular methods
                std::vector<std::string> args;
                for (size_t i = 0; i < param_types.size(); ++i) {
                    args.push_back(generate_json_extraction(param_types[i],
                                                            "params[" + std::to_string(i) + "]"));
                }

                if (return_type == "void") {
                    line("obj->" + m + "(" + join(args, ", ") + ");");
                    line("return success_response();");
                } else {
                    line("auto result = obj->" + m + "(" + join(args, ", ") + ");");
                    line("return success_response(" +
                         generate_json_conversion(return_type, "result") + ");");
                }
            }

            dedent();
        }

        if (!first) {
            line("} else {");
            indent();
            line("return error_response(\"Unknown method: \" + method, 404);");
            dedent();
            line("}");
        } else {
            line("return error_response(\"No methods available\", 404);");
        }

        dedent();
        line("} catch (const std::exception& e) {");
        indent();
        line("return error_response(e.what());");
        dedent();
        line("}");
        dedent();
        line("}");
        line();
    }

    // Write Rosetta-based method invocation for lambda methods
    void write_rosetta_method_invocation(const std::string              &cpp_type,
                                         const std::string              &method_name,
                                         const std::vector<std::string> &param_types,
                                         const std::string              &return_type) {
        line("std::vector<rosetta::Any> args;");
        if (!param_types.empty()) {
            line("args.reserve(" + std::to_string(param_types.size()) + ");");
        }
        for (size_t i = 0; i < param_types.size(); ++i) {
            std::string converted =
                generate_json_extraction(param_types[i], "params[" + std::to_string(i) + "]");
            line("args.emplace_back(" + converted + ");");
        }

        line("auto& meta = rosetta::Registry::instance().get<" + cpp_type + ">();");

        std::string norm_return = normalize_type(return_type);

        if (return_type == "void") {
            line("meta.invoke_method(*obj, \"" + method_name + "\", std::move(args));");
            line("return success_response();");
        } else {
            line("auto result = meta.invoke_method(*obj, \"" + method_name +
                 "\", std::move(args));");
            line("return success_response(" +
                 generate_json_conversion(return_type, "result.as<" + norm_return + ">()") + ");");
        }
    }

    void write_server_setup() {
        line("// ============================================================================");
        line("// REST Server Setup");
        line("// ============================================================================");
        line();
        line("void setup_routes(httplib::Server& server) {");
        indent();

        // CORS headers
        line("// Add CORS headers");
        line("server.set_default_headers({");
        indent();
        line("{\"Access-Control-Allow-Origin\", \"*\"},");
        line("{\"Access-Control-Allow-Methods\", \"GET, POST, PUT, DELETE, OPTIONS\"},");
        line("{\"Access-Control-Allow-Headers\", \"Content-Type\"}");
        dedent();
        line("});");
        line();

        // Health check
        line("// Health check");
        line("server.Get(\"/health\", [](const httplib::Request&, httplib::Response& res) {");
        indent();
        line("res.set_content(json{{\"status\", \"ok\"}, {\"module\", \"" + config_.module_name +
             "\"}}.dump(), \"application/json\");");
        dedent();
        line("});");
        line();

        // List all classes
        line("// List all registered classes");
        line("server.Get(\"/api/classes\", [](const httplib::Request&, httplib::Response& res) {");
        indent();
        line("json classes = json::array();");

        // List only non-abstract classes with their binding names
        auto &registry = rosetta::Registry::instance();
        for (const auto &name : registry.list_classes()) {
            if (config_.should_skip_class(name))
                continue;
            if (abstract_classes_.count(name) > 0)
                continue;

            std::string binding_name = get_binding_name(name);
            line("classes.push_back(\"" + binding_name + "\");");
        }

        line("res.set_content(success_response(classes).dump(), \"application/json\");");
        dedent();
        line("});");
        line();

        // Get class info
        line("// Get class information");
        line("server.Get(\"/api/classes/:name\", [](const httplib::Request& req, "
             "httplib::Response& res) {");
        indent();
        line("std::string name = req.path_params.at(\"name\");");

        bool first = true;
        for (const auto &name : registry.list_classes()) {
            if (config_.should_skip_class(name))
                continue;
            if (abstract_classes_.count(name) > 0)
                continue;

            std::string binding_name = get_binding_name(name);
            std::string handler_name = binding_name + "Handler";

            std::string cond = first ? "if" : "} else if";
            first            = false;
            line(cond + " (name == \"" + binding_name + "\") {");
            indent();
            line("res.set_content(success_response(" + handler_name +
                 "::get_info()).dump(), \"application/json\");");
            dedent();
        }
        if (!first) {
            line("} else {");
            indent();
            line("res.status = 404;");
            line("res.set_content(error_response(\"Class not found: \" + name, 404).dump(), "
                 "\"application/json\");");
            dedent();
            line("}");
        }
        dedent();
        line("});");
        line();

        // List all objects
        line("// List all created objects");
        line("server.Get(\"/api/objects\", [](const httplib::Request&, httplib::Response& res) {");
        indent();
        line("res.set_content(success_response(ObjectStore::instance().list_objects()).dump(), "
             "\"application/json\");");
        dedent();
        line("});");
        line();

        // Create object
        line("// Create a new object");
        line("server.Post(\"/api/objects/:class\", [](const httplib::Request& req, "
             "httplib::Response& res) {");
        indent();
        line("std::string cls = req.path_params.at(\"class\");");
        line("json params = req.body.empty() ? json::array() : json::parse(req.body);");
        line();

        first = true;
        for (const auto &name : registry.list_classes()) {
            if (config_.should_skip_class(name))
                continue;
            if (abstract_classes_.count(name) > 0)
                continue;

            std::string binding_name = get_binding_name(name);
            std::string handler_name = binding_name + "Handler";

            std::string cond = first ? "if" : "} else if";
            first            = false;
            line(cond + " (cls == \"" + binding_name + "\") {");
            indent();
            line("res.set_content(" + handler_name +
                 "::create(params).dump(), \"application/json\");");
            dedent();
        }
        if (!first) {
            line("} else {");
            indent();
            line("res.status = 404;");
            line("res.set_content(error_response(\"Unknown class: \" + cls, 404).dump(), "
                 "\"application/json\");");
            dedent();
            line("}");
        }
        dedent();
        line("});");
        line();

        // Call method on object
        line("// Call a method on an object");
        line("server.Post(\"/api/objects/:id/:method\", [](const httplib::Request& req, "
             "httplib::Response& res) {");
        indent();
        line("std::string id = req.path_params.at(\"id\");");
        line("std::string method = req.path_params.at(\"method\");");
        line("json params = req.body.empty() ? json::array() : json::parse(req.body);");
        line();
        line("std::string cls = ObjectStore::instance().get_class(id);");
        line("if (cls.empty()) {");
        indent();
        line("res.status = 404;");
        line("res.set_content(error_response(\"Object not found: \" + id, 404).dump(), "
             "\"application/json\");");
        line("return;");
        dedent();
        line("}");
        line();

        first = true;
        for (const auto &name : registry.list_classes()) {
            if (config_.should_skip_class(name))
                continue;
            if (abstract_classes_.count(name) > 0)
                continue;

            std::string binding_name = get_binding_name(name);
            std::string handler_name = binding_name + "Handler";

            std::string cond = first ? "if" : "} else if";
            first            = false;
            line(cond + " (cls == \"" + binding_name + "\") {");
            indent();
            line("res.set_content(" + handler_name +
                 "::call_method(id, method, params).dump(), \"application/json\");");
            dedent();
        }
        if (!first) {
            line("} else {");
            indent();
            line("res.status = 500;");
            line("res.set_content(error_response(\"Internal error\", 500).dump(), "
                 "\"application/json\");");
            dedent();
            line("}");
        }
        dedent();
        line("});");
        line();

        // Delete object
        line("// Delete an object");
        line("server.Delete(\"/api/objects/:id\", [](const httplib::Request& req, "
             "httplib::Response& res) {");
        indent();
        line("std::string id = req.path_params.at(\"id\");");
        line("if (ObjectStore::instance().remove(id)) {");
        indent();
        line("res.set_content(success_response().dump(), \"application/json\");");
        dedent();
        line("} else {");
        indent();
        line("res.status = 404;");
        line("res.set_content(error_response(\"Object not found: \" + id, 404).dump(), "
             "\"application/json\");");
        dedent();
        line("}");
        dedent();
        line("});");
        line();

        // OPTIONS handler for CORS preflight
        line("// CORS preflight handler");
        line("server.Options(\".*\", [](const httplib::Request&, httplib::Response& res) {");
        indent();
        line("res.status = 204;");
        dedent();
        line("});");

        dedent();
        line("}");
        line();
    }

    void write_main() {
        line("// ============================================================================");
        line("// Main Entry Point");
        line("// ============================================================================");
        line();
        line("int main(int argc, char* argv[]) {");
        indent();

        // Registration
        if (!config_.registration_function.empty()) {
            line("// Register classes with Rosetta");
            line(config_.get_registration_call() + ";");
            line();
        }

        line("// Parse command line");
        line("std::string host = \"0.0.0.0\";");
        line("int port = 8080;");
        line();
        line("for (int i = 1; i < argc; ++i) {");
        indent();
        line("std::string arg = argv[i];");
        line("if (arg == \"--port\" && i + 1 < argc) {");
        indent();
        line("port = std::stoi(argv[++i]);");
        dedent();
        line("} else if (arg == \"--host\" && i + 1 < argc) {");
        indent();
        line("host = argv[++i];");
        dedent();
        line("} else if (arg == \"--help\") {");
        indent();
        line("std::cout << \"Usage: \" << argv[0] << \" [--host HOST] [--port PORT]\\n\";");
        line("std::cout << \"  --host HOST  Bind address (default: 0.0.0.0)\\n\";");
        line("std::cout << \"  --port PORT  Listen port (default: 8080)\\n\";");
        line("return 0;");
        dedent();
        line("}");
        dedent();
        line("}");
        line();

        line("// Create and configure server");
        line("httplib::Server server;");
        line("setup_routes(server);");
        line();

        line("std::cout << \"" + config_.module_name + " REST API Server\\n\";");
        line("std::cout << \"Listening on \" << host << \":\" << port << \"\\n\";");
        line("std::cout << \"\\nEndpoints:\\n\";");
        line("std::cout << \"  GET  /health              - Health check\\n\";");
        line("std::cout << \"  GET  /api/classes         - List all classes\\n\";");
        line("std::cout << \"  GET  /api/classes/:name   - Get class info\\n\";");
        line("std::cout << \"  GET  /api/objects         - List all objects\\n\";");
        line("std::cout << \"  POST /api/objects/:class  - Create object\\n\";");
        line("std::cout << \"  POST /api/objects/:id/:method - Call method\\n\";");
        line("std::cout << \"  DELETE /api/objects/:id   - Delete object\\n\";");
        line("std::cout << \"\\nPress Ctrl+C to stop.\\n\\n\";");
        line();

        line("if (!server.listen(host.c_str(), port)) {");
        indent();
        line("std::cerr << \"Failed to start server on \" << host << \":\" << port << \"\\n\";");
        line("return 1;");
        dedent();
        line("}");
        line();
        line("return 0;");
        dedent();
        line("}");
    }

    // Normalize type string for code generation (full cleanup)
    std::string normalize_type(const std::string &t) const {
        std::string r = t;
        size_t      pos;

        // Remove std::__1:: (libc++ internal namespace)
        while ((pos = r.find("std::__1::")) != std::string::npos) {
            r.replace(pos, 10, "std::");
        }

        // Remove allocator template arguments
        size_t alloc_start;
        while ((alloc_start = r.find(", std::allocator<")) != std::string::npos) {
            size_t depth     = 1;
            size_t alloc_end = alloc_start + 17;
            while (alloc_end < r.size() && depth > 0) {
                if (r[alloc_end] == '<')
                    depth++;
                else if (r[alloc_end] == '>')
                    depth--;
                alloc_end++;
            }
            r.erase(alloc_start, alloc_end - alloc_start);
        }

        // Remove const (prefix and postfix forms)
        while ((pos = r.find("const ")) != std::string::npos)
            r.erase(pos, 6);
        while ((pos = r.find(" const")) != std::string::npos)
            r.erase(pos, 6);
        // Remove & and *
        while ((pos = r.find("&")) != std::string::npos)
            r.erase(pos, 1);
        while ((pos = r.find("*")) != std::string::npos)
            r.erase(pos, 1);

        // Trim whitespace
        while (!r.empty() && r[0] == ' ')
            r.erase(0, 1);
        while (!r.empty() && r.back() == ' ')
            r.pop_back();

        return r;
    }

    // Check if type is a std::function
    bool is_function_type(const std::string &type) const {
        std::string norm = normalize_type(type);
        return norm.find("function<") != std::string::npos ||
               norm.find("std::function<") != std::string::npos;
    }

    // Check if type is a pointer type
    bool is_pointer_type(const std::string &type) const {
        return type.find('*') != std::string::npos;
    }

    // Check if type is a registered class
    bool is_registered_class(const std::string &type) const {
        std::string norm     = normalize_type(type);
        auto       &registry = rosetta::Registry::instance();

        for (const auto &name : registry.list_classes()) {
            auto *holder = registry.get_by_name(name);
            if (holder) {
                std::string cpp_type = normalize_type(holder->get_cpp_type_name());
                if (norm == cpp_type || norm == name)
                    return true;

                size_t pos = cpp_type.rfind("::");
                if (pos != std::string::npos) {
                    std::string short_name = cpp_type.substr(pos + 2);
                    if (norm == short_name)
                        return true;
                }
            }
        }
        return false;
    }

    // Get the full C++ type name for a registered class
    std::string get_registered_class_cpp_type(const std::string &type) const {
        std::string norm     = normalize_type(type);
        auto       &registry = rosetta::Registry::instance();

        for (const auto &name : registry.list_classes()) {
            auto *holder = registry.get_by_name(name);
            if (holder) {
                std::string cpp_type = normalize_type(holder->get_cpp_type_name());
                if (norm == cpp_type || norm == name) {
                    return holder->get_cpp_type_name();
                }
                size_t pos = cpp_type.rfind("::");
                if (pos != std::string::npos) {
                    std::string short_name = cpp_type.substr(pos + 2);
                    if (norm == short_name) {
                        return holder->get_cpp_type_name();
                    }
                }
            }
        }
        return norm;
    }

    // Check if a method has any std::function parameters
    bool has_function_parameter(const rosetta::core::Registry::MetadataHolder *holder,
                                const std::string                             &method_name) const {
        auto info        = holder->get_method_info(method_name);
        auto param_types = info.get_param_types_str();
        for (const auto &pt : param_types) {
            if (is_function_type(pt))
                return true;
        }
        return false;
    }

    // Helper: Generate JSON extraction code
    std::string generate_json_extraction(const std::string &type,
                                         const std::string &json_expr) const {
        std::string norm   = normalize_type(type);
        bool        is_ptr = is_pointer_type(type);

        if (type.find("vector<double>") != std::string::npos) {
            return "json_to_vector_double(" + json_expr + ")";
        }
        if (type.find("vector<int>") != std::string::npos) {
            return "json_to_vector_int(" + json_expr + ")";
        }
        if (type.find("vector<float>") != std::string::npos) {
            return "json_to_vector_float(" + json_expr + ")";
        }
        if (type.find("string") != std::string::npos) {
            return json_expr + ".get<std::string>()";
        }
        if (norm == "bool") {
            return json_expr + ".get<bool>()";
        }
        if (norm == "int" || norm == "long" || norm == "size_t") {
            return json_expr + ".get<int>()";
        }
        if (norm == "float" || norm == "double") {
            return json_expr + ".get<double>()";
        }

        // For registered class types, expect an object ID string
        if (is_registered_class(norm)) {
            std::string full_type = get_registered_class_cpp_type(norm);
            if (is_ptr) {
                return "ObjectStore::instance().get<" + full_type + ">(" + json_expr +
                       ".get<std::string>()).get()";
            } else {
                return "*ObjectStore::instance().get<" + full_type + ">(" + json_expr +
                       ".get<std::string>())";
            }
        }

        if (is_function_type(norm)) {
            return "/* ERROR: std::function not supported via REST */";
        }

        return json_expr + ".get<" + norm + ">()";
    }

    // Helper: Generate JSON conversion code
    std::string generate_json_conversion(const std::string &type, const std::string &var) const {
        if (type.find("vector<double>") != std::string::npos) {
            return "vector_double_to_json(" + var + ")";
        }
        if (type.find("vector<int>") != std::string::npos) {
            return "vector_int_to_json(" + var + ")";
        }
        if (type.find("vector<float>") != std::string::npos) {
            return "vector_float_to_json(" + var + ")";
        }
        if (type.find("vector<") != std::string::npos) {
            return "json(" + var + ")";
        }
        return "json(" + var + ")";
    }
};