// examples/javascript/classes/binding.cxx
// Simple test for pointer handling

#include <rosetta/generators/js.h>

// Simple class A with one integer member
class A : public rosetta::Introspectable {
    INTROSPECTABLE(A)
public:
    A() : value_(0) {}
    A(int v) : value_(v) {}

    int  getValue() const { return value_; }
    void setValue(int v) { value_ = v; }

private:
    int value_;
};

void A::registerIntrospection(rosetta::TypeRegistrar<A> reg) {
    reg.constructor<>()
        .constructor<int>()
        .member("value", &A::value_)
        .method("getValue", &A::getValue)
        .method("setValues", &A::setValue);
}

// -----------------------------------------------------

// Class B that contains an A and returns pointer to it
class B : public rosetta::Introspectable {
    INTROSPECTABLE(B)
public:
    B() : a_(0) {}
    B(int v) : a_(v) {}

    A  *getA() { return &a_; }
    int getAValue() const { return a_.getValue(); }

private:
    A a_;
};

void B::registerIntrospection(rosetta::TypeRegistrar<B> reg) {
    reg.constructor<>()
        .constructor<int>()
        .method("getA", &B::getA)
        .method("getAValue", &B::getAValue);
}

// -----------------------------------------------------

REGISTER_TYPE(A);

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    rosetta::JsGenerator generator(env, exports);
    rosetta::registerPointerType<A>(generator);
    generator.bind_classes<A, B>();

    return exports;
}

NODE_API_MODULE(rosetta, Init)