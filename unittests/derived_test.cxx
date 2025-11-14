#include "TEST.h"

// ---------------------------------------------

class Interface {
public:
    virtual double tol() const = 0;
};

// ---------------------------------------------

class IAlgo {
public:
    virtual ~IAlgo() {}
    virtual void run() = 0;
};

// ---------------------------------------------

class Algo1 : public IAlgo {
public:
    void run() override { std::cerr << "Running Algo1\n"; }
};

// ---------------------------------------------

class Algo2 : public IAlgo, public Interface {
public:
    void   run() override { std::cerr << "Running Algo2 with tol " << tol() << std::endl; }
    double tol() const override { return (1. + std::sqrt(5)) / 2.; }
};

// ---------------------------------------------

class Algo11 : public Algo1 {
public:
    void run() override { std::cerr << "Running Algo11\n"; }
};

// ---------------------------------------------

TEST(Derived, basic) {
    ROSETTA_REGISTER_CLASS(Interface)
        .pure_virtual_method<double, void>("tol");

    ROSETTA_REGISTER_CLASS(IAlgo)
        .pure_virtual_method<void, void>("run");

    ROSETTA_REGISTER_CLASS(Algo1)
        .inherits_from<IAlgo>("IAlgo")
        .virtual_method("run", &Algo1::run);

    ROSETTA_REGISTER_CLASS(Algo2)
        .inherits_from<IAlgo>("IAlgo")
        .inherits_from<Interface>("Interface")
        .virtual_method("run", &Algo2::run);

    ROSETTA_REGISTER_CLASS(Algo11)
        .inherits_from<Algo1>("Algo1")
        .virtual_method("run", &Algo11::run);

    // --------------------------------------------------

    ROSETTA_GET_META(IAlgo).dump(std::cerr);
    ROSETTA_GET_META(Algo1).dump(std::cerr);
    ROSETTA_GET_META(Algo2).dump(std::cerr);
    ROSETTA_GET_META(Algo11).dump(std::cerr);

    // --------------------------------------------------

    auto   meta = ROSETTA_GET_META(Algo2);

    //=========================================
    // Classical
    //=========================================
    Algo2 a;
    meta.invoke_method(a, "run");

    //=========================================
    // Using pointer
    //=========================================
    IAlgo *ptra    = new Algo2();
    meta.invoke_method(ptra, "run"); delete ptra;

    //=========================================
    // Using shared_ptr
    //=========================================
    std::shared_ptr<IAlgo> sptra  = std::make_shared<Algo2>();
    meta.invoke_method(sptra, "run");
}

RUN_TESTS();
