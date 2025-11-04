#include <iostream>
#include <rosetta/generators/js/js_generator.h>
#include <rosetta/generators/js/type_converters.h>
#include <rosetta/rosetta.h>

using namespace rosetta;
using namespace rosetta::generators::js;

struct Point {
    double x, y, z;
};

struct Triangle {
    int a, b, c;
};

class Surface {
public:
    std::vector<double> positions;
    std::vector<int>    indices;

    Surface() = default;
    Surface(const std::vector<double> &positions, const std::vector<int> &indices)
        : positions(positions), indices(indices) {}
    // Surface(const Surface &other) : positions(other.positions), indices(other.indices) {}

    void forEachPoint(const std::function<void(const Point &)> &func) const {
        size_t num_points = positions.size() / 3;
        for (size_t i = 0; i < num_points; ++i) {
            func(Point{positions[3 * i], positions[3 * i + 1], positions[3 * i + 2]});
        }
    }
    void forEachTriangle(const std::function<void(const Triangle &)> &func) const {
        size_t num_triangles = indices.size() / 3;
        for (size_t i = 0; i < num_triangles; ++i) {
            func(Triangle{indices[3 * i], indices[3 * i + 1], indices[3 * i + 2]});
        }
    }

    // --- converters (get/set) exposed to Rosetta ---
    std::vector<Point> get_points() const {
        std::vector<Point> out;
        out.reserve(positions.size() / 3);
        forEachPoint([&](const Point &p) { out.push_back(p); });
        return out;
    }
    void set_points(const std::vector<Point> &pts) {
        positions.resize(3 * pts.size());
        for (size_t i = 0; i < pts.size(); ++i) {
            positions[3 * i + 0] = pts[i].x;
            positions[3 * i + 1] = pts[i].y;
            positions[3 * i + 2] = pts[i].z;
        }
    }
    std::vector<Triangle> get_triangles() const {
        std::vector<Triangle> out;
        out.reserve(indices.size() / 3);
        forEachTriangle([&](const Triangle &t) { out.push_back(t); });
        return out;
    }
    void set_triangles(const std::vector<Triangle> &tris) {
        indices.resize(3 * tris.size());
        for (size_t i = 0; i < tris.size(); ++i) {
            indices[3 * i + 0] = tris[i].a;
            indices[3 * i + 1] = tris[i].b;
            indices[3 * i + 2] = tris[i].c;
        }
    }
};

class Model {
    std::vector<Surface> surfaces;

public:
    Model() = default;

    void addSurface(const Surface &surface) { surfaces.push_back(surface); }

    size_t surface_count() const { return surfaces.size(); }

    // Expose getters/setters for binding (copy semantics for simplicity)
    Surface get_surface(size_t i) const { return surfaces.at(i); }
    void    set_surface(size_t i, const Surface &s) { surfaces.at(i) = s; }

    // Keep your existing iterator, too
    void forEachSurface(const std::function<void(const Surface &)> &func) const {
        for (const auto &s : surfaces)
            func(s);
    }
};

// ============================================================================
// ROSETTA REGISTRATION
// ============================================================================

static void register_classes() {
    // Point & Triangle (PODs)
    ROSETTA_REGISTER_CLASS(Point) // :contentReference[oaicite:1]{index=1}
        .constructor<>()          // ctors API
        .field("x", &Point::x)    // fields API
        .field("y", &Point::y)    // (all in ClassMetadata)
        .field("z", &Point::z);   // :contentReference[oaicite:2]{index=2}

    ROSETTA_REGISTER_CLASS(Triangle)
        .constructor<>()
        .field("a", &Triangle::a)
        .field("b", &Triangle::b)
        .field("c", &Triangle::c);

    // Surface: raw arrays + high-level converters
    ROSETTA_REGISTER_CLASS(Surface)
        .constructor<>()                                                      // default
        .constructor<const std::vector<double> &, const std::vector<int> &>() // parametric
        .field("positions", &Surface::positions)                              // expose raw data
        .field("indices", &Surface::indices)              // :contentReference[oaicite:3]{index=3}
        .method("get_points", &Surface::get_points)       // expose converters as methods
        .method("set_points", &Surface::set_points)       // method API
        .method("get_triangles", &Surface::get_triangles) // supports const & non-const
        .method("set_triangles", &Surface::set_triangles)
        .method("forEachPoint", &Surface::forEachPoint)
        .method("forEachTriangle",
                &Surface::forEachTriangle); // :contentReference[oaicite:4]{index=4}

    // Model: high-level accessors for surfaces
    ROSETTA_REGISTER_CLASS(Model)
        .constructor<>()
        .method("addSurface", &Model::addSurface)
        .method("surface_count", &Model::surface_count)
        .method("get_surface", &Model::get_surface)
        .method("set_surface", &Model::set_surface); // :contentReference[oaicite:5]{index=5}
}

// ============================================================================
// N-API BINDING WITH TYPE INFO
// ============================================================================

BEGIN_JS_MODULE(gen) {
    register_classes();
    register_vector_converter<int>(gen);
    register_vector_converter<double>(gen);

    // Register Point converter (C++ <-> JS)
    gen.register_converter<Point>(
        // C++ to JS
        [](Napi::Env env, const core::Any &val) -> Napi::Value {
            const auto  &p   = val.as<Point>();
            Napi::Object obj = Napi::Object::New(env);
            obj.Set("x", Napi::Number::New(env, p.x));
            obj.Set("y", Napi::Number::New(env, p.y));
            obj.Set("z", Napi::Number::New(env, p.z));
            return obj;
        },
        // JS to C++
        [](const Napi::Value &val) -> core::Any {
            if (!val.IsObject())
                return core::Any();
            Napi::Object obj = val.As<Napi::Object>();
            Point        p;
            p.x = obj.Get("x").As<Napi::Number>().DoubleValue();
            p.y = obj.Get("y").As<Napi::Number>().DoubleValue();
            p.z = obj.Get("z").As<Napi::Number>().DoubleValue();
            return core::Any(p);
        });

    // Register Triangle converter (C++ <-> JS)
    gen.register_converter<Triangle>(
        // C++ to JS
        [](Napi::Env env, const core::Any &val) -> Napi::Value {
            const auto  &t   = val.as<Triangle>();
            Napi::Object obj = Napi::Object::New(env);
            obj.Set("a", Napi::Number::New(env, t.a));
            obj.Set("b", Napi::Number::New(env, t.b));
            obj.Set("c", Napi::Number::New(env, t.c));
            return obj;
        },
        // JS to C++
        [](const Napi::Value &val) -> core::Any {
            if (!val.IsObject())
                return core::Any();
            Napi::Object obj = val.As<Napi::Object>();
            Triangle     t;
            t.a = obj.Get("a").As<Napi::Number>().Int32Value();
            t.b = obj.Get("b").As<Napi::Number>().Int32Value();
            t.c = obj.Get("c").As<Napi::Number>().Int32Value();
            return core::Any(t);
        });

    // Register std::function<void(const Point&)> converter (JS callback -> C++ function)
    gen.register_converter<std::function<void(const Point &)>>(
        // C++ to JS (not used in this case)
        [](Napi::Env env, const core::Any &val) -> Napi::Value {
            return env.Undefined();
        },
        // JS to C++ (the important part)
        [&gen](const Napi::Value &val) -> core::Any {
            if (!val.IsFunction()) {
                return core::Any();
            }

            // Create a persistent reference to the JS function
            Napi::FunctionReference js_func = Napi::Persistent(val.As<Napi::Function>());
            Napi::Env               env     = val.Env();

            // Create a C++ std::function that calls the JS callback
            std::function<void(const Point &)> cpp_func = [js_func, env, &gen](const Point &p) {
                // Convert Point to JS
                core::Any    any_point(p);
                Napi::Value  js_point = gen.any_to_js(env, any_point, nullptr);
                
                // Call JS function with the converted point
                js_func.Call({js_point});
            };

            return core::Any(cpp_func);
        });

    // Register std::function<void(const Triangle&)> converter
    gen.register_converter<std::function<void(const Triangle &)>>(
        // C++ to JS (not used)
        [](Napi::Env env, const core::Any &val) -> Napi::Value {
            return env.Undefined();
        },
        // JS to C++
        [&gen](const Napi::Value &val) -> core::Any {
            if (!val.IsFunction()) {
                return core::Any();
            }

            Napi::FunctionReference js_func = Napi::Persistent(val.As<Napi::Function>());
            Napi::Env               env     = val.Env();

            std::function<void(const Triangle &)> cpp_func = [js_func, env, &gen](const Triangle &t) {
                core::Any   any_triangle(t);
                Napi::Value js_triangle = gen.any_to_js(env, any_triangle, nullptr);
                js_func.Call({js_triangle});
            };

            return core::Any(cpp_func);
        });

    gen.bind_classes<Surface, Model>();
}
END_JS_MODULE();