#include <rosetta/extensions/generators/py_generator.h>
#include <rosetta/rosetta.h>

class Base {
public:
    virtual ~Base()    = default; // Virtual destructor is essential for polymorphic classes
    virtual void run() = 0;
};

class Derived : public Base {
public:
    void run() override { std::cerr << "Hello world!" << std::endl; }
};

BEGIN_PY_MODULE(rosetta_example, "Testing abstract class") {
    ROSETTA_REGISTER_CLASS(Base).pure_virtual_method("run", &Base::run);
    ROSETTA_REGISTER_CLASS(Derived).virtual_method("run", &Derived::run);
    
    BIND_PY_CLASS(Base);
    BIND_PY_DERIVED_CLASS(Derived, Base);
}
END_PY_MODULE()
