#include "TEST.h"

// Simple function with no arguments
void hello() {
    std::cout << "Hello, World!\n";
}

// Function with return value
int add(int a, int b) {
    return a + b;
}

// Function with multiple types
double calculate(int x, double y, float z) {
    return x + y + z;
}

// Function with string
std::string greet(const std::string &name, int age) {
    return "Hello, " + name + "! You are " + std::to_string(age) + " years old.";
}

// Complex function
double complex_calc(int a, double b, float c, long d) {
    return a * b + c - d;
}

TEST(functions, basic) {
    ROSETTA_REGISTER_FUNCTION(hello);
    ROSETTA_REGISTER_FUNCTION(add);
    ROSETTA_REGISTER_FUNCTION(calculate);
    ROSETTA_REGISTER_FUNCTION(greet);
    ROSETTA_REGISTER_FUNCTION(complex_calc);

    // List all registered functions
    std::cout << "Registered functions:\n";
    for (const auto &name : rosetta::FunctionRegistry::instance().list_functions()) {
        std::cout << "  - " << name << "\n";
    }
    std::cout << "\n";

    // Dump metadata for each function
    ROSETTA_GET_FUNCTION(hello).dump(std::cout);
    ROSETTA_GET_FUNCTION(add).dump(std::cout);
    ROSETTA_GET_FUNCTION(calculate).dump(std::cout);
    ROSETTA_GET_FUNCTION(greet).dump(std::cout);
    ROSETTA_GET_FUNCTION(complex_calc).dump(std::cout);

    // Invoke functions dynamically
    std::cout << "\n=== Dynamic Function Invocation ===\n\n";

    // Call hello()
    std::cout << "Calling hello():\n";
    rosetta::FunctionRegistry::instance().invoke("hello");

    // Call add(5, 3)
    std::cout << "\nCalling add(5, 3):\n";
    rosetta::Any result = rosetta::FunctionRegistry::instance().invoke("add", {5, 3});
    std::cout << "Result: " << result.as<int>() << "\n";
    EXPECT_EQ(result.as<int>(), 8);

    // Call calculate(10, 20.5, 5.5f)
    std::cout << "\nCalling calculate(10, 20.5, 5.5f):\n";
    result = rosetta::FunctionRegistry::instance().invoke("calculate", {10, 20.5, 5.5f});
    std::cout << "Result: " << result.as<double>() << "\n";
    EXPECT_EQ(result.as<double>(), 36);

    // Call greet("Alice", 30)
    std::cout << "\nCalling greet(\"Alice\", 30):\n";
    result = rosetta::FunctionRegistry::instance().invoke("greet", {"Alice", 30});
    std::cout << "Result: " << result.as<std::string>() << "\n";
    EXPECT_EQ(result.as<std::string>(), "Hello, Alice! You are 30 years old.");

    // Call complex_calc(5, 10.5, 2.5f, 3L)
    std::cout << "\nCalling complex_calc(5, 10.5, 2.5f, 3L):\n";
    result = rosetta::FunctionRegistry::instance().invoke("complex_calc", {5, 10.5, 2.5f, 3L});
    std::cout << "Result: " << result.as<double>() << "\n";
    EXPECT_EQ(result.as<double>(), 52);

    // Query function metadata
    std::cout << "\n=== Querying Function Metadata ===\n\n";
    auto &add_meta = ROSETTA_GET_FUNCTION(add);
    std::cout << "Function 'add' has " << add_meta.arity() << " parameters\n";
    std::cout << "Return type: " << rosetta::get_readable_type_name(add_meta.return_type()) << "\n";
    std::cout << "Parameter types:\n";
    for (size_t i = 0; i < add_meta.param_types().size(); ++i) {
        std::cout << "  [" << i << "] "
                  << rosetta::get_readable_type_name(add_meta.param_types()[i]) << "\n";
    }

    // Error handling
    std::cout << "\n=== Error Handling ===\n\n";
    try {
        // Wrong number of arguments
        rosetta::FunctionRegistry::instance().invoke("add", {5});
    } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << "\n";
    }

    try {
        // Non-existent function
        rosetta::FunctionRegistry::instance().invoke("non_existent");
    } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}

RUN_TESTS();