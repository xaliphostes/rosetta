#include <rosetta/generators/js.h>

class Algo {
public:
    double run(double tol) {
        // Long computation
        double       a = 0;
        const double N = 1000000;
        for (size_t i = 0; i < N; ++i) {
            a += std::sin(i / N * 3.1415926);
        }
        return a / N;
    }
};

// -----------------------------------------------------
// Wrapper around class IAlgo to have proper introspection
// -----------------------------------------------------
class IAlgo : public rosetta::Introspectable {
    INTROSPECTABLE(IAlgo)
public:
    double run(double tol) { return algo_.run(tol); }

private:
    Algo algo_;
};

void IAlgo::registerIntrospection(rosetta::TypeRegistrar<IAlgo> reg) {
    reg.constructor<>().method("run", &IAlgo::run);
}

// -----------------------------------------------------
// Binding in Js
// -----------------------------------------------------
BEGIN_JS(generator) {
    rosetta::bind_class<IAlgo>(generator, "Algo");
}
END_JS()
