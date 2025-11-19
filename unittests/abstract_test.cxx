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
    void run() override { std::cerr << "Hello world!" << std::endl; }
};

void initFct() {
    using namespace rosetta;

    // Vector3D
    ROSETTA_REGISTER_CLASS(Base1).pure_virtual_method<void>("run").method("help", &Base1::help);
    ROSETTA_REGISTER_CLASS(Base2).method("doit", &Base2::doit).method("hello", &Base2::hello);
    ROSETTA_REGISTER_CLASS(Derived)
        .inherits_from<Base1>("Base1")
        .inherits_from<Base2>("Base2")
        .virtual_method("run", &Derived::run);
    // .base_method<Base>("help", &Base::help); // No longer needed!
}

TEST(Abstract, basic) {
    initFct();

    auto &meta = ROSETTA_GET_META(Derived);
    meta.dump(std::cerr);

    // Static invoke
    Derived d;
    meta.invoke_method(d, "run");
    meta.invoke_method(d, "help");
    meta.invoke_method(d, "hello", {rosetta::Any(5), rosetta::Any(std::string("coucou"))});
    meta.invoke_method(d, "doit");
}

RUN_TESTS();