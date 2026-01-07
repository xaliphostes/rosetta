/*
 * Test file for lambda_constructor registration
 *
 * Demonstrates adding custom constructors to classes using lambdas.
 * This is useful when you need to transform input types (e.g., array -> Matrix)
 * or provide factory-like construction patterns.
 */
#include "TEST.h"
#include <array>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

using namespace rosetta;

// ============================================================================
// Example: A 3x3 symmetric matrix (like stress/strain tensors)
// ============================================================================
struct Matrix33 {
    double xx, yy, zz, xy, xz, yz;

    Matrix33() : xx(0), yy(0), zz(0), xy(0), xz(0), yz(0) {}
    Matrix33(double _xx, double _yy, double _zz, double _xy, double _xz, double _yz)
        : xx(_xx), yy(_yy), zz(_zz), xy(_xy), xz(_xz), yz(_yz) {}

    double trace() const { return xx + yy + zz; }
};

// ============================================================================
// Example: A class that takes a callback returning Matrix33
// Similar to arch::UserRemote
// ============================================================================
class UserCallback {
public:
    using Callback = std::function<Matrix33(double, double, double)>;

    UserCallback(Callback cb, bool flag = true) : callback_(cb), flag_(flag) {}

    Matrix33 evaluate(double x, double y, double z) const { return callback_(x, y, z); }

    bool getFlag() const { return flag_; }

private:
    Callback callback_;
    bool     flag_;
};

// ============================================================================
// TEST: Basic lambda_constructor registration and metadata
// ============================================================================
TEST(LambdaConstructor, basic_registration) {
    // Register Matrix33 first
    ROSETTA_REGISTER_CLASS(Matrix33)
        .constructor<>()
        .constructor<double, double, double, double, double, double>()
        .field("xx", &Matrix33::xx)
        .field("yy", &Matrix33::yy)
        .field("zz", &Matrix33::zz)
        .field("xy", &Matrix33::xy)
        .field("xz", &Matrix33::xz)
        .field("yz", &Matrix33::yz)
        .method("trace", &Matrix33::trace);

    // Register UserCallback with a lambda constructor
    // This allows Python users to pass a function returning vector<double> instead of Matrix33
    ROSETTA_REGISTER_CLASS(UserCallback)
        // Original constructor (for reference)
        .constructor<std::function<Matrix33(double, double, double)>, bool>()

        // Lambda constructor: converts vector<double> callback to Matrix33 callback
        .lambda_constructor<std::function<std::vector<double>(double, double, double)>, bool>(
            "[](std::function<std::vector<double>(double, double, double)> userFct, bool flag) {"
            "    auto wrapper = [userFct](double x, double y, double z) -> Matrix33 {"
            "        auto v = userFct(x, y, z);"
            "        return Matrix33(v[0], v[1], v[2], v[3], v[4], v[5]);"
            "    };"
            "    return UserCallback(wrapper, flag);"
            "}")
        .method("getFlag", &UserCallback::getFlag);

    auto &meta = ROSETTA_GET_META(UserCallback);
    meta.dump(std::cerr);

    // Verify constructors are registered
    auto ctors = meta.constructor_infos();
    EXPECT_EQ(ctors.size(), 2u);

    // First constructor is regular
    EXPECT_FALSE(ctors[0].is_lambda);
    EXPECT_EQ(ctors[0].arity, 2u);

    // Second constructor is lambda
    EXPECT_TRUE(ctors[1].is_lambda);
    EXPECT_EQ(ctors[1].arity, 2u);
    EXPECT_TRUE(ctors[1].lambda_body.find("userFct") != std::string::npos);
    EXPECT_TRUE(ctors[1].lambda_body.find("Matrix33") != std::string::npos);

    std::cout << "Lambda constructor basic registration: PASSED" << std::endl;
}

// ============================================================================
// TEST: Lambda constructor parameter types
// ============================================================================
TEST(LambdaConstructor, parameter_types) {
    class SimpleClass {
    public:
        SimpleClass(int x, double y, const std::string &s) : x_(x), y_(y), s_(s) {}
        int         getX() const { return x_; }
        double      getY() const { return y_; }
        std::string getS() const { return s_; }

    private:
        int         x_;
        double      y_;
        std::string s_;
    };

    ROSETTA_REGISTER_CLASS(SimpleClass)
        .constructor<int, double, const std::string &>()
        // Lambda constructor with different parameter types
        .lambda_constructor<std::vector<int>, std::string>(
            "[](std::vector<int> values, std::string name) {"
            "    int sum = 0;"
            "    for (int v : values) sum += v;"
            "    return SimpleClass(sum, static_cast<double>(values.size()), name);"
            "}")
        .method("getX", &SimpleClass::getX)
        .method("getY", &SimpleClass::getY)
        .method("getS", &SimpleClass::getS);

    auto &meta  = ROSETTA_GET_META(SimpleClass);
    auto  ctors = meta.constructor_infos();

    EXPECT_EQ(ctors.size(), 2u);

    // Check lambda constructor has correct parameter types
    EXPECT_TRUE(ctors[1].is_lambda);
    EXPECT_EQ(ctors[1].arity, 2u);
    EXPECT_EQ(ctors[1].param_types.size(), 2u);

    // First param should be vector<int>
    std::string param0_type = demangle(ctors[1].param_types[0].name());
    EXPECT_TRUE(param0_type.find("vector") != std::string::npos);

    // Second param should be string
    std::string param1_type = demangle(ctors[1].param_types[1].name());
    EXPECT_TRUE(param1_type.find("string") != std::string::npos);

    std::cout << "Lambda constructor parameter types: PASSED" << std::endl;
}

// ============================================================================
// TEST: Multiple lambda constructors
// ============================================================================
TEST(LambdaConstructor, multiple_lambda_ctors) {
    class Flexible {
    public:
        Flexible(double value) : value_(value) {}
        double getValue() const { return value_; }

    private:
        double value_;
    };

    ROSETTA_REGISTER_CLASS(Flexible)
        .constructor<double>()
        // Lambda from int
        .lambda_constructor<int>(
            "[](int x) { return Flexible(static_cast<double>(x)); }")
        // Lambda from vector (sum)
        .lambda_constructor<std::vector<double>>(
            "[](std::vector<double> v) {"
            "    double sum = 0;"
            "    for (double x : v) sum += x;"
            "    return Flexible(sum);"
            "}")
        // Lambda from two ints
        .lambda_constructor<int, int>(
            "[](int a, int b) { return Flexible(static_cast<double>(a + b)); }")
        .method("getValue", &Flexible::getValue);

    auto &meta  = ROSETTA_GET_META(Flexible);
    auto  ctors = meta.constructor_infos();

    EXPECT_EQ(ctors.size(), 4u);
    EXPECT_FALSE(ctors[0].is_lambda); // Regular ctor
    EXPECT_TRUE(ctors[1].is_lambda);  // From int
    EXPECT_TRUE(ctors[2].is_lambda);  // From vector
    EXPECT_TRUE(ctors[3].is_lambda);  // From two ints

    // Check arities
    EXPECT_EQ(ctors[0].arity, 1u);
    EXPECT_EQ(ctors[1].arity, 1u);
    EXPECT_EQ(ctors[2].arity, 1u);
    EXPECT_EQ(ctors[3].arity, 2u);

    std::cout << "Multiple lambda constructors: PASSED" << std::endl;
}

// ============================================================================
// TEST: Registry access to lambda constructor metadata
// ============================================================================
TEST(LambdaConstructor, registry_access) {
    class Wrapped {
    public:
        Wrapped(std::array<double, 6> arr)
            : data_{arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]} {}
        double sum() const {
            double s = 0;
            for (double v : data_)
                s += v;
            return s;
        }

    private:
        std::array<double, 6> data_;
    };

    ROSETTA_REGISTER_CLASS(Wrapped)
        .constructor<std::array<double, 6>>()
        // Lambda constructor from vector<double>
        .lambda_constructor<std::vector<double>>(
            "[](std::vector<double> v) {"
            "    std::array<double, 6> arr;"
            "    for (size_t i = 0; i < 6 && i < v.size(); ++i) arr[i] = v[i];"
            "    return Wrapped(arr);"
            "}")
        .method("sum", &Wrapped::sum);

    // Access via Registry
    auto &registry = core::Registry::instance();
    auto *holder   = registry.get_by_name("Wrapped");
    EXPECT_TRUE(holder != nullptr);

    auto ctors = holder->get_constructors();
    EXPECT_EQ(ctors.size(), 2u);

    // Check lambda constructor through type-erased interface
    EXPECT_FALSE(ctors[0].is_lambda);
    EXPECT_TRUE(ctors[1].is_lambda);
    EXPECT_FALSE(ctors[1].lambda_body.empty());

    // Verify parameter types via get_param_types()
    auto params = ctors[1].get_param_types();
    EXPECT_EQ(params.size(), 1u);
    EXPECT_TRUE(params[0].find("vector") != std::string::npos);

    std::cout << "Registry access to lambda constructor: PASSED" << std::endl;
}

// ============================================================================
// TEST: Lambda body content verification
// ============================================================================
TEST(LambdaConstructor, lambda_body_content) {
    class Converter {
    public:
        Converter(double x, double y) : x_(x), y_(y) {}
        double getX() const { return x_; }
        double getY() const { return y_; }

    private:
        double x_, y_;
    };

    const char *lambdaBody =
        "[](std::pair<double, double> p) {"
        "    return Converter(p.first, p.second);"
        "}";

    ROSETTA_REGISTER_CLASS(Converter)
        .constructor<double, double>()
        .lambda_constructor<std::pair<double, double>>(lambdaBody)
        .method("getX", &Converter::getX)
        .method("getY", &Converter::getY);

    auto &meta  = ROSETTA_GET_META(Converter);
    auto  ctors = meta.constructor_infos();

    EXPECT_TRUE(ctors[1].is_lambda);
    EXPECT_EQ(ctors[1].lambda_body, std::string(lambdaBody));

    // Verify the body contains expected substrings
    EXPECT_TRUE(ctors[1].lambda_body.find("std::pair") != std::string::npos);
    EXPECT_TRUE(ctors[1].lambda_body.find("p.first") != std::string::npos);
    EXPECT_TRUE(ctors[1].lambda_body.find("p.second") != std::string::npos);
    EXPECT_TRUE(ctors[1].lambda_body.find("Converter") != std::string::npos);

    std::cout << "Lambda body content verification: PASSED" << std::endl;
}

RUN_TESTS();

/* Expected Output:

=== Rosetta metadata for class: UserCallback ===
Constructors:
  - UserCallback(std::function<Matrix33(double, double, double)>, bool)
  - UserCallback(std::function<std::vector<double>(double, double, double)>, bool) [lambda]
...

Lambda constructor basic registration: PASSED
Lambda constructor parameter types: PASSED
Multiple lambda constructors: PASSED
Registry access to lambda constructor: PASSED
Lambda body content verification: PASSED

All tests passed!
*/
