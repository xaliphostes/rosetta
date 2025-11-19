#include "TEST.h"

class Base1 {
public:
    virtual ~Base1() {}
    virtual void run() = 0;

    // Check if a methd on base class can be call from a derived class
    void help() const { std::cerr << "help from Base1 class" << std::endl; }
};

class Base2 {
public:
    void   doit() const { std::cerr << "doit from Base2 class" << std::endl; }
    double hello(double d, const std::string &s) {
        std::cerr << "hello from Base2 class with values " << d << " and " << s << std::endl;
        return 0;
    }
};

class Derived : public Base1, public Base2 {
public:
    void        run() override { std::cerr << "Hello world!" << std::endl; }
    std::string name() const { return "Hector"; }
};

void initFct() {
    using namespace rosetta;

    // Vector3D
    ROSETTA_REGISTER_CLASS(Base1).pure_virtual_method<void>("run").method("help", &Base1::help);
    ROSETTA_REGISTER_CLASS(Base2).method("doit", &Base2::doit).method("hello", &Base2::hello);
    ROSETTA_REGISTER_CLASS(Derived)
        .inherits_from<Base1>("Base1")
        .inherits_from<Base2>("Base2")
        .override_method("run", &Derived::run)
        .method("name", &Derived::name);
}

TEST(Abstract, basic) {
    initFct();

    auto &meta = ROSETTA_GET_META(Derived);
    meta.dump(std::cerr);

    Derived d;
    meta.invoke_method(d, "run");                    // from Base1
    meta.invoke_method(d, "help");                   // from Base1
    meta.invoke_method(d, "doit");                   // from Base2
    meta.invoke_method(d, "hello", {5.0, "coucou"}); // from Base2
    meta.invoke_method(d, "name");                   // from Derived
}

RUN_TESTS();