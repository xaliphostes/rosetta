#include <rosetta/generators/js.h>

// -----------------------------------------------------

class A : public rosetta::Introspectable {
    INTROSPECTABLE(A)
public:
    A() { }
    A(int x)
        : x(x)
    {
    }
    int getX() const { return x; }
    void setX(int value) { x = value; }

private:
    int x { 0 };
};
REGISTER_TYPE(A);

void A::registerIntrospection(rosetta::TypeRegistrar<A> reg)
{
    reg.constructor<>()
        .constructor<int>()
        .member("x", &A::x)
        .method("getX", &A::getX)
        .method("setX", &A::setX);
}

// -----------------------------------------------------

class B : public rosetta::Introspectable {
    INTROSPECTABLE(B)
public:
    B() { a_ = new A(); }
    B(int x) { a_ = new A(x); }
    A* a() { return a_; }
    void setA(A* a) { this->a_ = a; }

private:
    A* a_;
};

void B::registerIntrospection(rosetta::TypeRegistrar<B> reg)
{
    reg.constructor<>().constructor<int>().method("a", &B::a).method("setA", &B::setA);
}

// -----------------------------------------------------

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    rosetta::JsGenerator(env, exports).bind_classes<A, B>();
    return exports;
}

NODE_API_MODULE(rosetta, Init)
