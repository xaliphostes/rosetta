#include "TEST.h"

/**
 * Test the retrun type for getter
 *
 * Test the argument type for setter
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
    for (const auto &method_name : meta.methods()) {
        size_t          arity     = meta.get_method_arity(method_name);
        const auto     &arg_types = meta.get_method_arg_types(method_name);
        std::type_index ret_type  = meta.get_method_return_type(method_name);

        std::string method;

        // Print method signature
        method += rosetta::get_readable_type_name(ret_type) + " " + method_name + "(";
        for (size_t i = 0; i < arg_types.size(); ++i) {
            method += rosetta::get_readable_type_name(arg_types[i]);
            if (i < arg_types.size() - 1)
                method += ", ";
        }
        method += ")";

        all_methods.push_back(method);
    }
    
    for (const auto& method: all_methods) {
        std::cerr << method << std::endl;
    }

    EXPECT_EQ(all_methods.size(), 2);
    EXPECT_NOT_EQ(all_methods[0], all_methods[1]);
}

RUN_TESTS();