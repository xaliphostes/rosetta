#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace rosetta {

    // ============================================================================
    // Core Type Erasure
    // ============================================================================

    class Any {
        struct Holder {
            virtual ~Holder()                     = default;
            virtual Holder     *clone() const     = 0;
            virtual std::string type_name() const = 0;
        };

        template <typename T> struct HolderImpl : Holder {
            T value;
            HolderImpl(T v) : value(std::move(v)) {}
            Holder     *clone() const override { return new HolderImpl(value); }
            std::string type_name() const override { return typeid(T).name(); }
        };

        std::unique_ptr<Holder> holder_;

    public:
        Any() = default;

        template <typename T> Any(T value) : holder_(new HolderImpl<T>(std::move(value))) {}

        Any(const Any &other) : holder_(other.holder_ ? other.holder_->clone() : nullptr) {}
        Any(Any &&)                 = default;
        Any &operator=(const Any &) = default;
        Any &operator=(Any &&)      = default;

        template <typename T> T &as() { return static_cast<HolderImpl<T> *>(holder_.get())->value; }

        template <typename T> const T &as() const {
            return static_cast<const HolderImpl<T> *>(holder_.get())->value;
        }

        std::string type_name() const { return holder_ ? holder_->type_name() : "empty"; }
    };

    // ============================================================================
    // Type Info pour les bindings
    // ============================================================================

    enum class TypeKind { Void, Bool, Int, Float, Double, String, Object, Array };

    struct TypeInfo {
        TypeKind    kind;
        std::string name;
        std::string class_name; // Pour les objets

        static TypeInfo from_cpp_type(const std::string &cpp_type) {
            if (cpp_type.find("void") != std::string::npos)
                return {TypeKind::Void, "void", ""};
            if (cpp_type.find("bool") != std::string::npos)
                return {TypeKind::Bool, "bool", ""};
            if (cpp_type.find("int") != std::string::npos)
                return {TypeKind::Int, "int", ""};
            if (cpp_type.find("float") != std::string::npos)
                return {TypeKind::Float, "float", ""};
            if (cpp_type.find("double") != std::string::npos)
                return {TypeKind::Double, "double", ""};
            if (cpp_type.find("string") != std::string::npos)
                return {TypeKind::String, "string", ""};
            return {TypeKind::Object, cpp_type, cpp_type};
        }
    };

    // ============================================================================
    // Member Descriptors avec metadata
    // ============================================================================

    struct MemberInfo {
        std::string name;
        TypeInfo    type;
        bool        is_const = false;
    };

    template <typename Class, typename T> struct Field {
        MemberInfo info;
        T Class::*ptr;

        Field(std::string name, T Class::*p) : ptr(p) {
            info.name = std::move(name);
            info.type = TypeInfo::from_cpp_type(typeid(T).name());
        }

        T       &get(Class &obj) const { return obj.*ptr; }
        const T &get(const Class &obj) const { return obj.*ptr; }
        void     set(Class &obj, const T &value) const { obj.*ptr = value; }
    };

    struct MethodInfo {
        std::string           name;
        TypeInfo              return_type;
        std::vector<TypeInfo> param_types;
        bool                  is_const = false;
    };

    template <typename Class, typename Ret, typename... Args> struct Method {
        MethodInfo info;
        Ret (Class::*ptr)(Args...);

        Method(std::string name, Ret (Class::*p)(Args...)) : ptr(p) {
            info.name        = std::move(name);
            info.return_type = TypeInfo::from_cpp_type(typeid(Ret).name());
            (info.param_types.push_back(TypeInfo::from_cpp_type(typeid(Args).name())), ...);
        }

        Ret invoke(Class &obj, Args... args) const {
            return (obj.*ptr)(std::forward<Args>(args)...);
        }
    };

    // ============================================================================
    // Class Metadata
    // ============================================================================

    template <typename Class> class ClassMetadata {
        std::string                                                        name_;
        std::unordered_map<std::string, std::function<Any(Class &)>>       field_getters_;
        std::unordered_map<std::string, std::function<void(Class &, Any)>> field_setters_;
        std::unordered_map<std::string, std::function<Any(Class &, std::vector<Any>)>> methods_;
        std::unordered_map<std::string, MemberInfo>                                    field_infos_;
        std::unordered_map<std::string, MethodInfo> method_infos_;
        std::vector<std::string>                    field_names_;
        std::vector<std::string>                    method_names_;

    public:
        ClassMetadata(std::string name) : name_(std::move(name)) {}

        const std::string &name() const { return name_; }

        template <typename T> ClassMetadata &field(const std::string &name, T Class::*ptr) {
            field_names_.push_back(name);

            MemberInfo info;
            info.name          = name;
            info.type          = TypeInfo::from_cpp_type(typeid(T).name());
            field_infos_[name] = info;

            field_getters_[name] = [ptr](Class &obj) -> Any { return Any(obj.*ptr); };

            field_setters_[name] = [ptr](Class &obj, Any value) { obj.*ptr = value.as<T>(); };

            return *this;
        }

        template <typename Ret, typename... Args>
        ClassMetadata &method(const std::string &name, Ret (Class::*ptr)(Args...)) {
            method_names_.push_back(name);

            MethodInfo info;
            info.name        = name;
            info.return_type = TypeInfo::from_cpp_type(typeid(Ret).name());
            (info.param_types.push_back(TypeInfo::from_cpp_type(typeid(Args).name())), ...);
            method_infos_[name] = info;

            methods_[name] = [ptr](Class &obj, std::vector<Any> args) -> Any {
                if constexpr (sizeof...(Args) == 0) {
                    if constexpr (std::is_void_v<Ret>) {
                        (obj.*ptr)();
                        return Any(0);
                    } else {
                        return Any((obj.*ptr)());
                    }
                } else {
                    return invoke_with_args(obj, ptr, args, std::index_sequence_for<Args...>{});
                }
            };

            return *this;
        }

        const std::vector<std::string> &fields() const { return field_names_; }
        const std::vector<std::string> &methods() const { return method_names_; }

        const MemberInfo &get_field_info(const std::string &name) const {
            return field_infos_.at(name);
        }

        const MethodInfo &get_method_info(const std::string &name) const {
            return method_infos_.at(name);
        }

        Any get_field(Class &obj, const std::string &name) const {
            return field_getters_.at(name)(obj);
        }

        void set_field(Class &obj, const std::string &name, Any value) const {
            field_setters_.at(name)(obj, std::move(value));
        }

        Any invoke_method(Class &obj, const std::string &name, std::vector<Any> args = {}) const {
            return methods_.at(name)(obj, std::move(args));
        }

    private:
        template <typename Ret, typename... Args, size_t... Is>
        static Any invoke_with_args(Class &obj, Ret (Class::*ptr)(Args...), std::vector<Any> &args,
                                    std::index_sequence<Is...>) {
            if constexpr (std::is_void_v<Ret>) {
                (obj.*ptr)(args[Is].as<Args>()...);
                return Any(0);
            } else {
                return Any((obj.*ptr)(args[Is].as<Args>()...));
            }
        }
    };

    // ============================================================================
    // Registry
    // ============================================================================

    class Registry {
        struct MetadataHolder {
            virtual ~MetadataHolder()                            = default;
            virtual std::string              get_name() const    = 0;
            virtual std::vector<std::string> get_fields() const  = 0;
            virtual std::vector<std::string> get_methods() const = 0;
        };

        template <typename Class> struct MetadataHolderImpl : MetadataHolder {
            ClassMetadata<Class> metadata;
            MetadataHolderImpl(std::string name) : metadata(std::move(name)) {}

            std::string              get_name() const override { return metadata.name(); }
            std::vector<std::string> get_fields() const override { return metadata.fields(); }
            std::vector<std::string> get_methods() const override { return metadata.methods(); }
        };

        std::unordered_map<std::string, std::unique_ptr<MetadataHolder>> classes_;

        Registry() = default;

    public:
        static Registry &instance() {
            static Registry reg;
            return reg;
        }

        template <typename Class> ClassMetadata<Class> &register_class(const std::string &name) {
            auto  holder   = std::make_unique<MetadataHolderImpl<Class>>(name);
            auto *ptr      = &holder->metadata;
            classes_[name] = std::move(holder);
            return *ptr;
        }

        template <typename Class> const ClassMetadata<Class> &get() const {
            for (const auto &[name, holder] : classes_) {
                if (auto *typed = dynamic_cast<MetadataHolderImpl<Class> *>(holder.get())) {
                    return typed->metadata;
                }
            }
            throw std::runtime_error("Class not registered");
        }

        std::vector<std::string> list_classes() const {
            std::vector<std::string> names;
            for (const auto &[name, _] : classes_) {
                names.push_back(name);
            }
            return names;
        }

        const MetadataHolder *get_by_name(const std::string &name) const {
            auto it = classes_.find(name);
            return it != classes_.end() ? it->second.get() : nullptr;
        }
    };

    // ============================================================================
    // BINDING GENERATORS
    // ============================================================================

    class BindingGenerator {
    public:
        virtual ~BindingGenerator()          = default;
        virtual std::string generate() const = 0;
    };

    // Python binding generator (pybind11 style)
    class PythonBindingGenerator : public BindingGenerator {
    public:
        std::string generate() const override {
            std::stringstream ss;
            ss << "#include <pybind11/pybind11.h>\n";
            ss << "#include \"rosetta.h\"\n\n";
            ss << "namespace py = pybind11;\n\n";
            ss << "PYBIND11_MODULE(my_module, m) {\n";

            for (const auto &class_name : Registry::instance().list_classes()) {
                auto *holder = Registry::instance().get_by_name(class_name);
                if (!holder)
                    continue;

                ss << "    py::class_<" << class_name << ">(m, \"" << class_name << "\")\n";
                ss << "        .def(py::init<>())\n";

                // Fields
                for (const auto &field : holder->get_fields()) {
                    ss << "        .def_readwrite(\"" << field << "\", &" << class_name
                       << "::" << field << ")\n";
                }

                // Methods
                for (const auto &method : holder->get_methods()) {
                    ss << "        .def(\"" << method << "\", &" << class_name << "::" << method
                       << ")\n";
                }

                ss << "        ;\n\n";
            }

            ss << "}\n";
            return ss.str();
        }
    };

    // JavaScript binding generator (Emscripten style)
    class JavaScriptBindingGenerator : public BindingGenerator {
    public:
        std::string generate() const override {
            std::stringstream ss;
            ss << "#include <emscripten/bind.h>\n";
            ss << "#include \"rosetta.h\"\n\n";
            ss << "using namespace emscripten;\n\n";
            ss << "EMSCRIPTEN_BINDINGS(my_module) {\n";

            for (const auto &class_name : Registry::instance().list_classes()) {
                auto *holder = Registry::instance().get_by_name(class_name);
                if (!holder)
                    continue;

                ss << "    class_<" << class_name << ">(\"" << class_name << "\")\n";
                ss << "        .constructor<>()\n";

                // Fields
                for (const auto &field : holder->get_fields()) {
                    ss << "        .property(\"" << field << "\", &" << class_name << "::" << field
                       << ")\n";
                }

                // Methods
                for (const auto &method : holder->get_methods()) {
                    ss << "        .function(\"" << method << "\", &" << class_name
                       << "::" << method << ")\n";
                }

                ss << "        ;\n\n";
            }

            ss << "}\n";
            return ss.str();
        }
    };

    // TypeScript declaration generator
    class TypeScriptBindingGenerator : public BindingGenerator {
    public:
        std::string generate() const override {
            std::stringstream ss;
            ss << "// Auto-generated TypeScript definitions\n\n";

            for (const auto &class_name : Registry::instance().list_classes()) {
                auto *holder = Registry::instance().get_by_name(class_name);
                if (!holder)
                    continue;

                ss << "export class " << class_name << " {\n";
                ss << "    constructor();\n\n";

                // Fields (simplifié)
                for (const auto &field : holder->get_fields()) {
                    ss << "    " << field << ": number; // TODO: proper type\n";
                }

                ss << "\n";

                // Methods (simplifié)
                for (const auto &method : holder->get_methods()) {
                    ss << "    " << method << "(): any; // TODO: proper signature\n";
                }

                ss << "}\n\n";
            }

            return ss.str();
        }
    };

    // Generic JSON schema generator (pour documentation ou validation)
    class JSONSchemaGenerator : public BindingGenerator {
    public:
        std::string generate() const override {
            std::stringstream ss;
            ss << "{\n";
            ss << "  \"classes\": [\n";

            bool first_class = true;
            for (const auto &class_name : Registry::instance().list_classes()) {
                auto *holder = Registry::instance().get_by_name(class_name);
                if (!holder)
                    continue;

                if (!first_class)
                    ss << ",\n";
                first_class = false;

                ss << "    {\n";
                ss << "      \"name\": \"" << class_name << "\",\n";
                ss << "      \"fields\": [";

                bool first_field = true;
                for (const auto &field : holder->get_fields()) {
                    if (!first_field)
                        ss << ", ";
                    first_field = false;
                    ss << "\"" << field << "\"";
                }

                ss << "],\n";
                ss << "      \"methods\": [";

                bool first_method = true;
                for (const auto &method : holder->get_methods()) {
                    if (!first_method)
                        ss << ", ";
                    first_method = false;
                    ss << "\"" << method << "\"";
                }

                ss << "]\n";
                ss << "    }";
            }

            ss << "\n  ]\n";
            ss << "}\n";
            return ss.str();
        }
    };

    // ============================================================================
    // EXEMPLE D'UTILISATION
    // ============================================================================

    class Vector3D {
    public:
        double x, y, z;

        Vector3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

        double length() const { return std::sqrt(x * x + y * y + z * z); }

        void normalize() {
            double len = length();
            if (len > 0) {
                x /= len;
                y /= len;
                z /= len;
            }
        }
    };

    class Point2D {
    public:
        double x, y;

        Point2D(double x = 0, double y = 0) : x(x), y(y) {}

        double distance() const { return std::sqrt(x * x + y * y); }
    };

    void register_types() {
        // Enregistrement simple
        auto &vec_meta = rosetta::Registry::instance().register_class<Vector3D>("Vector3D");
        vec_meta.field("x", &Vector3D::x)
            .field("y", &Vector3D::y)
            .field("z", &Vector3D::z)
            .method("length", &Vector3D::length)
            .method("normalize", &Vector3D::normalize);

        auto &pt_meta = rosetta::Registry::instance().register_class<Point2D>("Point2D");
        pt_meta.field("x", &Point2D::x)
            .field("y", &Point2D::y)
            .method("distance", &Point2D::distance);
    }

} // namespace rosetta

int main() {
    using namespace rosetta;

    // Enregistrer les types
    register_types();

    std::cout << "=== Registered Classes ===\n";
    for (const auto &name : Registry::instance().list_classes()) {
        std::cout << "  - " << name << "\n";
    }

    std::cout << "\n=== Python Binding (pybind11) ===\n";
    PythonBindingGenerator py_gen;
    std::cout << py_gen.generate() << "\n";

    std::cout << "\n=== JavaScript Binding (Emscripten) ===\n";
    JavaScriptBindingGenerator js_gen;
    std::cout << js_gen.generate() << "\n";

    std::cout << "\n=== TypeScript Definitions ===\n";
    TypeScriptBindingGenerator ts_gen;
    std::cout << ts_gen.generate() << "\n";

    std::cout << "\n=== JSON Schema ===\n";
    JSONSchemaGenerator json_gen;
    std::cout << json_gen.generate() << "\n";

    return 0;
}