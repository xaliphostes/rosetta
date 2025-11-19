#include "TEST.h"

class Base {
public:
    virtual void run() = 0;

    // Check if a methd on base class can be call from a derived class
    void help() const {
        std::cerr << "help from base class" << std::endl ;
    }
};

class Derived: public Base {
public:
    void run() override {
        std::cerr << "Hello world!" << std::endl;
    }
};

void initFct() {
    using namespace rosetta;

    // Vector3D
    ROSETTA_REGISTER_CLASS(Base)
        .pure_virtual_method<void>("run")
        .method("help", &Base::help);

    ROSETTA_REGISTER_CLASS(Derived)
        .inherits_from<Base>("Base")
        .virtual_method("run", &Derived::run)
        .base_method<Base>("help", &Base::help); // Have to fix! Better to skip this.
}

TEST(Abstract, basic) {
    initFct();
    
    std::cerr << "starting meta\n";
    auto    &meta = ROSETTA_GET_META(Derived);
    std::cerr << "ending meta\n";
    meta.dump(std::cerr);

    // Static invoke
    Derived d;
    meta.invoke_method(d, "run");
    meta.invoke_method(d, "help");
}

RUN_TESTS();