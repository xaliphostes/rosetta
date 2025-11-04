// ============================================================================
// functor_example.cpp
//
// Example showing how to bind C++ functors (callable objects) to JavaScript
// Demonstrates various functor patterns: simple functors, stateful functors,
// and functors with different signatures
// ============================================================================

#include <cmath>
#include <functional>
#include <iostream>
#include <rosetta/generators/js/js_generator.h>
#include <rosetta/generators/js/type_converters.h>
#include <rosetta/rosetta.h>

using namespace rosetta;
using namespace rosetta::generators::js;

// ============================================================================
// EXAMPLE FUNCTORS
// ============================================================================

/**
 * @brief Simple mathematical functor - squares a number
 */
class Squarer {
public:
    double operator()(double x) const { return x * x; }

    std::string name() const { return "Squarer"; }
};

/**
 * @brief Stateful functor - accumulates values
 */
class Accumulator {
private:
    double sum_;
    int    count_;

public:
    Accumulator() : sum_(0.0), count_(0) {}

    double operator()(double value) {
        sum_ += value;
        count_++;
        return sum_;
    }

    double get_sum() const { return sum_; }
    double get_average() const { return count_ > 0 ? sum_ / count_ : 0.0; }
    int    get_count() const { return count_; }

    void reset() {
        sum_   = 0.0;
        count_ = 0;
    }
};

/**
 * @brief Transformation functor - applies a custom transformation
 */
class VectorTransform {
private:
    double scale_;
    double offset_;

public:
    VectorTransform() : scale_(1.0), offset_(0.0) {}
    VectorTransform(double scale, double offset) : scale_(scale), offset_(offset) {}

    std::vector<double> operator()(const std::vector<double> &input) const {
        std::vector<double> result;
        result.reserve(input.size());
        for (double val : input) {
            result.push_back(val * scale_ + offset_);
        }
        return result;
    }

    double get_scale() const { return scale_; }
    void   set_scale(double s) { scale_ = s; }

    double get_offset() const { return offset_; }
    void   set_offset(double o) { offset_ = o; }
};

/**
 * @brief Predicate functor - tests conditions
 */
class RangePredicate {
private:
    double min_;
    double max_;

public:
    RangePredicate() : min_(0.0), max_(100.0) {}
    RangePredicate(double min, double max) : min_(min), max_(max) {}

    bool operator()(double value) const { return value >= min_ && value <= max_; }

    // Filter a vector based on predicate
    std::vector<double> filter(const std::vector<double> &input) const {
        std::vector<double> result;
        for (double val : input) {
            if ((*this)(val)) {
                result.push_back(val);
            }
        }
        return result;
    }

    double get_min() const { return min_; }
    void   set_min(double m) { min_ = m; }

    double get_max() const { return max_; }
    void   set_max(double m) { max_ = m; }
};

/**
 * @brief Binary operation functor
 */
class BinaryOp {
public:
    enum OpType { Add, Subtract, Multiply, Divide, Power };

private:
    OpType op_;

public:
    BinaryOp() : op_(Add) {}
    explicit BinaryOp(OpType op) : op_(op) {}

    double operator()(double a, double b) const {
        switch (op_) {
        case Add:
            return a + b;
        case Subtract:
            return a - b;
        case Multiply:
            return a * b;
        case Divide:
            return b != 0 ? a / b : 0.0;
        case Power:
            return std::pow(a, b);
        }
        return 0.0;
    }

    // Apply operation to two vectors element-wise
    std::vector<double> apply(const std::vector<double> &a, const std::vector<double> &b) const {
        size_t              size = std::min(a.size(), b.size());
        std::vector<double> result;
        result.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            result.push_back((*this)(a[i], b[i]));
        }
        return result;
    }

    int  get_operation() const { return static_cast<int>(op_); }
    void set_operation(int op) { op_ = static_cast<OpType>(op); }

    std::string get_operation_name() const {
        switch (op_) {
        case Add:
            return "Add";
        case Subtract:
            return "Subtract";
        case Multiply:
            return "Multiply";
        case Divide:
            return "Divide";
        case Power:
            return "Power";
        }
        return "Unknown";
    }
};

/**
 * @brief Composition functor - composes two functions
 */
class Compositor {
private:
    double scale1_;
    double offset1_;
    double scale2_;
    double offset2_;

public:
    Compositor() : scale1_(1.0), offset1_(0.0), scale2_(1.0), offset2_(0.0) {}

    // f(g(x)) where g(x) = x * scale1 + offset1, f(x) = x * scale2 + offset2
    double operator()(double x) const {
        double intermediate = x * scale1_ + offset1_;
        return intermediate * scale2_ + offset2_;
    }

    void set_inner(double scale, double offset) {
        scale1_  = scale;
        offset1_ = offset;
    }

    void set_outer(double scale, double offset) {
        scale2_  = scale;
        offset2_ = offset;
    }

    double get_scale1() const { return scale1_; }
    double get_offset1() const { return offset1_; }
    double get_scale2() const { return scale2_; }
    double get_offset2() const { return offset2_; }
};

// ============================================================================
// ROSETTA REGISTRATION
// ============================================================================

void register_functors() {
    // Register Squarer
    ROSETTA_REGISTER_CLASS(Squarer)
        .method("call", static_cast<double (Squarer::*)(double) const>(&Squarer::operator()))
        .method("name", &Squarer::name);

    // Register Accumulator
    ROSETTA_REGISTER_CLASS(Accumulator)
        .method("call", static_cast<double (Accumulator::*)(double)>(&Accumulator::operator()))
        .method("get_sum", &Accumulator::get_sum)
        .method("get_average", &Accumulator::get_average)
        .method("get_count", &Accumulator::get_count)
        .method("reset", &Accumulator::reset);

    // Register VectorTransform
    ROSETTA_REGISTER_CLASS(VectorTransform)
        .method("call", &VectorTransform::operator())
        .method("get_scale", &VectorTransform::get_scale)
        .method("set_scale", &VectorTransform::set_scale)
        .method("get_offset", &VectorTransform::get_offset)
        .method("set_offset", &VectorTransform::set_offset);

    // Register RangePredicate
    ROSETTA_REGISTER_CLASS(RangePredicate)
        .method("test",
                static_cast<bool (RangePredicate::*)(double) const>(&RangePredicate::operator()))
        .method("filter", &RangePredicate::filter)
        .method("get_min", &RangePredicate::get_min)
        .method("set_min", &RangePredicate::set_min)
        .method("get_max", &RangePredicate::get_max)
        .method("set_max", &RangePredicate::set_max);

    // Register BinaryOp
    ROSETTA_REGISTER_CLASS(BinaryOp)
        .method("call",
                static_cast<double (BinaryOp::*)(double, double) const>(&BinaryOp::operator()))
        .method("apply", &BinaryOp::apply)
        .method("get_operation", &BinaryOp::get_operation)
        .method("set_operation", &BinaryOp::set_operation)
        .method("get_operation_name", &BinaryOp::get_operation_name);

    // Register Compositor
    ROSETTA_REGISTER_CLASS(Compositor)
        .method("call", static_cast<double (Compositor::*)(double) const>(&Compositor::operator()))
        .method("set_inner", &Compositor::set_inner)
        .method("set_outer", &Compositor::set_outer)
        .method("get_scale1", &Compositor::get_scale1)
        .method("get_offset1", &Compositor::get_offset1)
        .method("get_scale2", &Compositor::get_scale2)
        .method("get_offset2", &Compositor::get_offset2);
}

// ============================================================================
// N-API BINDING
// ============================================================================

BEGIN_JS_MODULE(gen) {
    auto test_squarer = [](const Napi::CallbackInfo &info) -> Napi::Value {
        Napi::Env env = info.Env();

        std::cerr << "[DEBUG] test_squarer called with " << info.Length() << " arguments"
                  << std::endl;

        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Expected a number").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        double input = info[0].As<Napi::Number>().DoubleValue();
        std::cerr << "[DEBUG] Input value: " << input << std::endl;

        double result = input * input;
        std::cerr << "[DEBUG] Result: " << result << std::endl;

        return Napi::Number::New(env, result);
    };

    gen.exports.Set("testSquarer", Napi::Function::New(gen.env, test_squarer, "testSquarer"));

    // Register classes
    register_functors();

    // Register type converters
    register_vector_converter<double>(gen);
    register_vector_converter<int>(gen);

    // Bind all functor classes
    gen.bind_classes<Squarer, Accumulator, VectorTransform, RangePredicate, BinaryOp, Compositor>();

    // Add utility functions
    gen.add_utilities();
}
END_JS_MODULE();