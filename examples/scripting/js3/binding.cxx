#include <rosetta/generators/js.h>
#include <rosetta/generators/js_vectors.h>

using Vertices = std::vector<double>;
using Triangles = std::vector<size_t>;

REGISTER_TYPE_ALIAS_MANGLED(Vertices);
REGISTER_TYPE_ALIAS_MANGLED(Triangles);

class Surface : public rosetta::Introspectable {
    INTROSPECTABLE(Surface)
public:
    Surface() { }
    Surface(const std::vector<double>& v, const std::vector<size_t>& t)
        : vertices_(v)
        , triangles_(t)
    {
    }

    const Vertices& vertices() const { return vertices_; }
    void setVertices(const Vertices& v) { vertices_ = v; }
    const Triangles& triangles() const { return triangles_; }

private:
    Vertices vertices_;
    Triangles triangles_;
};

void Surface::registerIntrospection(rosetta::TypeRegistrar<Surface> reg)
{
    reg.constructor<>()
        .constructor<const std::vector<double>&, const std::vector<size_t>&>()
        .method("vertices", &Surface::vertices)
        .method("setVertices", &Surface::setVertices)
        .method("triangles", &Surface::triangles);
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    rosetta::JsGenerator generator(env, exports);
    rosetta::registerCommonVectorTypes(generator);
    rosetta::bind_class<Surface>(generator);

    return exports;
}

NODE_API_MODULE(js3, Init)
