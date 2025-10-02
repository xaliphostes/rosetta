#include <rosetta/generators/js.h>
#include <rosetta/generators/js_vectors.h>

class Surface : public rosetta::Introspectable {
    INTROSPECTABLE(Surface)
public:
    Surface() { }
    Surface(const std::vector<double>& v, const std::vector<size_t>& t)
        : vertices_(v)
        , triangles_(t)
    {
    }

    const std::vector<double>& vertices() const { return vertices_; }
    const std::vector<size_t>& triangles() const { return triangles_; }

private:
    std::vector<double> vertices_;
    std::vector<size_t> triangles_;
};

void Surface::registerIntrospection(rosetta::TypeRegistrar<Surface> reg)
{
    reg.constructor<>()
        .constructor<const std::vector<double>&, const std::vector<size_t>&>()
        .method("vertices", &Surface::vertices)
        .method("triangles", &Surface::triangles);
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    rosetta::JsGenerator generator(env, exports);
    rosetta::registerCommonVectorTypes(generator);
    generator.bind_class<Surface>();

    return exports;
}

NODE_API_MODULE(js3, Init)
