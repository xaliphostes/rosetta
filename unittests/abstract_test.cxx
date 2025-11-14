#include "TEST.h"

class Base {
public:
    virtual void run() = 0;
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
        .pure_virtual_method<void, void>("run");

    ROSETTA_REGISTER_CLASS(Derived)
        .virtual_method("run", &Derived::run);
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
}

RUN_TESTS();