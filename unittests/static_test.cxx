#include "TEST.h"

class A {
public:
    static std::string message() {
        return "Hello World!";
    }

    static double pi() {
        return 3.1415926;
    }
};

void initFct() {
    using namespace rosetta;

    // Vector3D
    ROSETTA_REGISTER_CLASS(A)
        .static_method("pi", &A::pi)
        .static_method("message", &A::message);
}

TEST(Static, basic) {
    initFct();
    
    std::cerr << "starting meta\n";
    auto    &meta = ROSETTA_GET_META(A);
    std::cerr << "ending meta\n";
    meta.dump(std::cerr);

    // Static invoke
    auto pi = meta.invoke_static_method("pi");
    EXPECT_NEAR(pi.as<double>(), 3.1415926, 1e-6);

    // Instance invoke
    A a;
    auto msg = meta.invoke_method(a, "message");
    EXPECT_STREQ(msg.as<std::string>(), "Hello World!");
}

RUN_TESTS();