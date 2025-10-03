#include <rosetta/generators/js.h>

using Stress = std::array<double, 6>;
REGISTER_TYPE_ALIAS_MANGLED(Stress);

class A : public rosetta::Introspectable {
    INTROSPECTABLE(A)
public:
    const Stress stress() const { return stress_; }
    void setStress(const Stress& stress) { stress_ = stress; }

private:
    Stress stress_;
};

void A::registerIntrospection(rosetta::TypeRegistrar<A> reg)
{
    reg.method("stress", &A::stress).method("setStress", &A::setStress);
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    rosetta::JsGenerator generator(env, exports);
    rosetta::registerCommonArrayTypes(generator);
    rosetta::bind_class<A>(generator);

    return exports;
}

NODE_API_MODULE(rosetta, Init)
