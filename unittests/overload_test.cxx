#include "TEST.h"

/**
 * Test the return type for getter
 * Test the argument type for setter
 *
 * Updated to use the new method_info() function
 */
class A {
public:
    void run(double tol) { std::cerr << "running with tol " << tol << std::endl; }
    void run(const std::string &msg) { std::cerr << "running with msg " << msg << std::endl; }
};

TEST(Overload, basic) {
    ROSETTA_REGISTER_CLASS(A)
        .method("run", static_cast<void (A::*)(double)>(&A::run))
        .method("run", static_cast<void (A::*)(const std::string &)>(&A::run));

    auto &meta = ROSETTA_GET_META(A);
    meta.dump(std::cerr);

    std::cout << "All methods:" << std::endl;
    std::vector<std::string> all_methods;

    // NEW APPROACH: Use method_info() to get all overloads correctly
    for (const auto &method_name : meta.methods()) {
        // Get all overloads for this method
        const auto &overloads = meta.method_info(method_name);

        // Iterate through each overload
        for (const auto &info : overloads) {
            std::string method;

            // Print method signature
            method += rosetta::get_readable_type_name(info.return_type) + " " + method_name + "(";

            for (size_t i = 0; i < info.arg_types.size(); ++i) {
                method += rosetta::get_readable_type_name(info.arg_types[i]);
                if (i < info.arg_types.size() - 1)
                    method += ", ";
            }
            method += ")";

            all_methods.push_back(method);
        }
    }

    for (const auto &method : all_methods) {
        std::cerr << method << std::endl;
    }

    // Now the tests should pass!
    EXPECT_EQ(all_methods.size(), 2);              // Should have exactly 2 methods
    EXPECT_NOT_EQ(all_methods[0], all_methods[1]); // They should be different

    // Verify the signatures
    EXPECT_TRUE(all_methods[0].find("double") != std::string::npos ||
                all_methods[0].find("string") != std::string::npos);
    EXPECT_TRUE(all_methods[1].find("double") != std::string::npos ||
                all_methods[1].find("string") != std::string::npos);
}

TEST(Overload, multiple_overloads) {
    // Test with more complex overloading
    class Calculator {
    public:
        int    compute(int x) { return x * 2; }
        double compute(double x) { return x * 2.5; }
        int    compute(int x, int y) { return x + y; }
        double compute(double x, double y) { return x + y; }
    };

    ROSETTA_REGISTER_CLASS(Calculator)
        .method("compute", static_cast<int (Calculator::*)(int)>(&Calculator::compute))
        .method("compute", static_cast<double (Calculator::*)(double)>(&Calculator::compute))
        .method("compute", static_cast<int (Calculator::*)(int, int)>(&Calculator::compute))
        .method("compute",
                static_cast<double (Calculator::*)(double, double)>(&Calculator::compute));

    auto &meta = ROSETTA_GET_META(Calculator);

    // Use method_info to get all overloads
    const auto &overloads = meta.method_info("compute");

    std::cout << "\nCalculator::compute has " << overloads.size() << " overload(s):" << std::endl;

    EXPECT_EQ(overloads.size(), 4); // Should have 4 overloads

    // Check arities
    size_t arity_1_count = 0;
    size_t arity_2_count = 0;

    for (const auto &info : overloads) {
        std::string sig = rosetta::get_readable_type_name(info.return_type) + " compute(";
        for (size_t i = 0; i < info.arg_types.size(); ++i) {
            sig += rosetta::get_readable_type_name(info.arg_types[i]);
            if (i < info.arg_types.size() - 1)
                sig += ", ";
        }
        sig += ")";

        std::cout << "  " << sig << std::endl;

        if (info.arity == 1)
            arity_1_count++;
        if (info.arity == 2)
            arity_2_count++;
    }

    EXPECT_EQ(arity_1_count, 2); // Two single-parameter overloads
    EXPECT_EQ(arity_2_count, 2); // Two two-parameter overloads
}

TEST(Overload, method_info_error_handling) {
    class Empty {};

    ROSETTA_REGISTER_CLASS(Empty);

    auto &meta = ROSETTA_GET_META(Empty);

    // Test that accessing non-existent method throws
    bool threw_exception = false;
    try {
        const auto &info = meta.method_info("nonexistent");
    } catch (const std::runtime_error &e) {
        threw_exception = true;
        std::cout << "Expected error: " << e.what() << std::endl;
    }

    EXPECT_TRUE(threw_exception);
}

RUN_TESTS();

/* Expected Output:

=== Rosetta metadata for class: A ===
Instantiable: true
Constructors (0):
Fields (0):
Methods (2):
  - void run(double) [1 arg]
  - void run(string) [1 arg]
Inheritance flags:
  is_abstract            = false
  is_polymorphic         = false
  has_virtual_destructor = false
  base_count             = 0
===============================================
All methods:
void run(double)
void run(string)

Calculator::compute has 4 overload(s):
  int compute(int)
  double compute(double)
  int compute(int, int)
  double compute(double, double)

Expected error: Method not found: nonexistent

All tests passed!
*/