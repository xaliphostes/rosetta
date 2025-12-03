/*
 * Test file for lambda_method registration
 *
 * Demonstrates adding synthetic methods to classes using lambdas.
 */
#include "TEST.h"
#include <iostream>
#include <string>

using namespace rosetta;

// ============================================================================
// Example: A solver class that we want to extend with synthetic methods
// ============================================================================
class Seidel {
public:
    Seidel() : tolerance_(1e-6), maxIterations_(100), lastResult_(0.0) {}

    void   setTolerance(double tol) { tolerance_ = tol; }
    double getTolerance() const { return tolerance_; }

    void setMaxIterations(int max) { maxIterations_ = max; }
    int  getMaxIterations() const { return maxIterations_; }

    void run() {
        std::cerr << "Running Seidel solver with tol=" << tolerance_
                  << ", maxIter=" << maxIterations_ << std::endl;
        lastResult_ = 42.0; // Simulated result
    }

    double getLastResult() const { return lastResult_; }

private:
    double tolerance_;
    int    maxIterations_;
    double lastResult_;
};

// ============================================================================
// TEST: Basic lambda method registration
// ============================================================================
TEST(LambdaMethod, basic_registration) {
    ROSETTA_REGISTER_CLASS_AS(Seidel, "Solver")
        // Regular methods
        .method("setTolerance", &Seidel::setTolerance)
        .method("getTolerance", &Seidel::getTolerance)
        .method("setMaxIterations", &Seidel::setMaxIterations)
        .method("getMaxIterations", &Seidel::getMaxIterations)
        .method("run", &Seidel::run)
        .method("getLastResult", &Seidel::getLastResult)

        // NEW: Synthetic method using lambda!
        // This method doesn't exist in the Seidel class, but we're adding it via reflection
        .lambda_method<void, double, int>("runWithParams",
                                          [](Seidel &self, double tol, int maxiter) {
                                              self.setTolerance(tol);
                                              self.setMaxIterations(maxiter);
                                              self.run();
                                          })

        // Another synthetic method - a convenience wrapper
        .lambda_method<void>("runDefault",
                             [](Seidel &self) {
                                 self.setTolerance(1e-8);
                                 self.setMaxIterations(1000);
                                 self.run();
                             })

        // Const lambda method - for read-only operations
        .lambda_method_const<std::string>("getStatus", [](const Seidel &self) -> std::string {
            return "Solver ready, tol=" + std::to_string(self.getTolerance());
        });

    auto &meta = ROSETTA_GET_META(Seidel);
    meta.dump(std::cerr);

    // Verify all methods are registered
    auto methods = meta.methods();
    EXPECT_TRUE(std::find(methods.begin(), methods.end(), "runWithParams") != methods.end());
    EXPECT_TRUE(std::find(methods.begin(), methods.end(), "runDefault") != methods.end());
    EXPECT_TRUE(std::find(methods.begin(), methods.end(), "getStatus") != methods.end());

    // Test invoking the lambda methods
    Seidel solver;

    // Invoke runWithParams(0.001, 500)
    meta.invoke_method(solver, "runWithParams", {0.001, 500});
    EXPECT_EQ(solver.getTolerance(), 0.001);
    EXPECT_EQ(solver.getMaxIterations(), 500);

    // Invoke runDefault()
    meta.invoke_method(solver, "runDefault");
    EXPECT_EQ(solver.getTolerance(), 1e-8);
    EXPECT_EQ(solver.getMaxIterations(), 1000);

    // Invoke const method getStatus()
    Any         result = meta.invoke_method(solver, "getStatus");
    std::string status = result.as<std::string>();
    std::cout << "Status: " << status << std::endl;
    EXPECT_TRUE(status.find("Solver ready") != std::string::npos);

    std::cout << "Lambda method registration: PASSED" << std::endl;
}

// ============================================================================
// TEST: Lambda methods with return values
// ============================================================================
TEST(LambdaMethod, return_values) {
    class Calculator {
    public:
        int value = 0;
    };

    ROSETTA_REGISTER_CLASS(Calculator)
        .field("value", &Calculator::value)

        // Lambda that returns a value
        .lambda_method<int, int>("addAndGet",
                                 [](Calculator &self, int x) -> int {
                                     self.value += x;
                                     return self.value;
                                 })

        // Lambda that returns computed value without modifying state
        .lambda_method_const<int, int, int>(
            "computeSum",
            [](const Calculator &self, int a, int b) -> int { return self.value + a + b; });

    auto &meta = ROSETTA_GET_META(Calculator);

    Calculator calc;
    calc.value = 10;

    // Test addAndGet
    Any result1 = meta.invoke_method(calc, "addAndGet", {5});
    EXPECT_EQ(result1.as<int>(), 15);
    EXPECT_EQ(calc.value, 15);

    // Test computeSum (const, shouldn't modify)
    Any result2 = meta.invoke_method(calc, "computeSum", {3, 7});
    EXPECT_EQ(result2.as<int>(), 25); // 15 + 3 + 7
    EXPECT_EQ(calc.value, 15);        // Unchanged

    std::cout << "Lambda return values: PASSED" << std::endl;
}

// ============================================================================
// TEST: Capturing lambdas
// ============================================================================
TEST(LambdaMethod, capturing_lambdas) {
    class Logger {
    public:
        std::string name;
    };

    // External configuration that the lambda captures
    std::string prefix   = "[LOG] ";
    int         logLevel = 2;

    ROSETTA_REGISTER_CLASS(Logger)
        .field("name", &Logger::name)

        // Lambda that captures external variables
        .lambda_method<std::string, std::string>(
            "formatMessage", [prefix, logLevel](Logger &self, std::string msg) -> std::string {
                return prefix + "[" + std::to_string(logLevel) + "] " + self.name + ": " + msg;
            });

    auto &meta = ROSETTA_GET_META(Logger);

    Logger logger;
    logger.name = "MyApp";

    Any         result = meta.invoke_method(logger, "formatMessage", {"Hello!"});
    std::string formatted = result.as<std::string>();

    std::cout << "Formatted: " << formatted << std::endl;
    EXPECT_TRUE(formatted.find("[LOG]") != std::string::npos);
    EXPECT_TRUE(formatted.find("[2]") != std::string::npos);
    EXPECT_TRUE(formatted.find("MyApp") != std::string::npos);

    std::cout << "Capturing lambdas: PASSED" << std::endl;
}

// ============================================================================
// TEST: Using std::function instead of lambda
// ============================================================================
TEST(LambdaMethod, std_function) {
    class Processor {
    public:
        int data = 0;
    };

    // Define a std::function
    std::function<int(Processor &, int)> processFunc = [](Processor &self, int x) -> int {
        self.data = x * 2;
        return self.data;
    };

    ROSETTA_REGISTER_CLASS(Processor)
        .field("data", &Processor::data)
        .lambda_method<int, int>("process", processFunc);

    auto &meta = ROSETTA_GET_META(Processor);

    Processor proc;
    Any       result = meta.invoke_method(proc, "process", {21});

    EXPECT_EQ(result.as<int>(), 42);
    EXPECT_EQ(proc.data, 42);

    std::cout << "std::function support: PASSED" << std::endl;
}

// ============================================================================
// TEST: Combining regular methods and lambda methods
// ============================================================================
TEST(LambdaMethod, combined_usage) {
    class BemSurface {
    public:
        void setBcType(int axis, int type) {
            std::cerr << "setBcType(int, int): " << axis << ", " << type << std::endl;
            axisInt_ = axis;
            typeInt_ = type;
        }

        void setBcType(const std::string &axis, const std::string &type) {
            std::cerr << "setBcType(string, string): " << axis << ", " << type << std::endl;
            axisStr_ = axis;
            typeStr_ = type;
        }

        int         axisInt_ = 0, typeInt_ = 0;
        std::string axisStr_, typeStr_;
    };

    ROSETTA_REGISTER_CLASS(BemSurface)
        // Regular overloaded methods (using the overload helpers)
        .method("setBcType", ROSETTA_OVERLOAD(BemSurface, void, setBcType, int, int))
        .method("setBcType", ROSETTA_OVERLOAD(BemSurface, void, setBcType, const std::string &,
                                              const std::string &))

        // Synthetic convenience method
        .lambda_method<void>("setDefaultBc",
                             [](BemSurface &self) {
                                 self.setBcType(0, 1);
                                 self.setBcType("x", "dirichlet");
                             })

        // Synthetic method to set both at once
        .lambda_method<void, int, int, const std::string &, const std::string &>(
            "setBcTypeBoth", [](BemSurface &self, int axisI, int typeI, const std::string &axisS,
                                const std::string &typeS) {
                self.setBcType(axisI, typeI);
                self.setBcType(axisS, typeS);
            });

    auto &meta = ROSETTA_GET_META(BemSurface);
    meta.dump(std::cerr);

    BemSurface surface;

    // Call the synthetic method
    meta.invoke_method(surface, "setDefaultBc");

    EXPECT_EQ(surface.axisInt_, 0);
    EXPECT_EQ(surface.typeInt_, 1);
    EXPECT_EQ(surface.axisStr_, "x");
    EXPECT_EQ(surface.typeStr_, "dirichlet");

    std::cout << "Combined usage: PASSED" << std::endl;
}

RUN_TESTS();

/* Expected Output:

=== Rosetta metadata for class: Solver ===
Methods:
  - void setTolerance(double)
  - double getTolerance()
  - ...
  - void runWithParams(double, int)     <-- Synthetic!
  - void runDefault()                    <-- Synthetic!
  - string getStatus() const             <-- Synthetic!
...

Running Seidel solver with tol=0.001, maxIter=500
Running Seidel solver with tol=1e-08, maxIter=1000
Status: Solver ready, tol=0.000000
Lambda method registration: PASSED
Lambda return values: PASSED
Formatted: [LOG] [2] MyApp: Hello!
Capturing lambdas: PASSED
std::function support: PASSED
setBcType(int, int): 0, 1
setBcType(string, string): x, dirichlet
Combined usage: PASSED

All tests passed!
*/