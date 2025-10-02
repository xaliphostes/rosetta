#include <rosetta/generators/js.h>

using Vertices = std::vector<double>;
using Triangles = std::vector<size_t>;

class Surface : public rosetta::Introspectable {
    INTROSPECTABLE(Surface)
public:
    Surface() { }
    Surface(const Vertices& v, const Triangles& t)
        : vertices_(v)
        , triangles_(t)
    {
    }

    const Vertices& vertices() const { return vertices_; }
    const Triangles& triangles() const { return triangles_; }

private:
    Vertices vertices_;
    Triangles triangles_;
};
REGISTER_TYPE(Vertices);
REGISTER_TYPE(Triangles);

void Surface::registerIntrospection(rosetta::TypeRegistrar<Surface> reg)
{
    reg.constructor<>()
        .constructor<const Vertices &, const Triangles &>()
        .method("vertices", &Surface::vertices)
        .method("triangles", &Surface::triangles);
}

// -----------------------------------------------------

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    rosetta::JsGenerator(env, exports).bind_class<Surface>();
    return exports;
}

NODE_API_MODULE(js3, Init)
