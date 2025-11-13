#include "TEST.h"

/**
 * Test the retrun type for getter
 * 
 * Test the argument type for setter
 */
class A {
public:
    double        getA() const { return a_; }
    const double &getB() const { return b_; }

    void setA(const double &a) { a_ = a; }
    void setB(double b) { b_ = b; }

private:
    double a_{0}, b_{0};
};

TEST(Setter, basic) {
    ROSETTA_REGISTER_CLASS(A)
        .property("a", &A::getA, &A::setA)
        .property("b", &A::getB, &A::setB);

    std::cerr << "starting meta\n";
    auto &meta = ROSETTA_GET_META(A);
    std::cerr << "ending meta\n";
    meta.dump(std::cerr);
}

RUN_TESTS();