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
REGISTER_TYPE(A);

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

BEGIN_JS(generator) {
    rosetta::registerPointerType<A>(generator);
    rosetta::bind_classes<A, B>(generator);
}
END_JS()
