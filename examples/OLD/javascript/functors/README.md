# Rosetta Functor Bindings

## Overview

This example demonstrates how to bind C++ functors (callable objects) to JavaScript using Rosetta's introspection system. Functors are objects that can be called like functions using `operator()`.

## What are Functors?

A **functor** (or function object) is a C++ class that implements `operator()`, making instances callable like functions:

```cpp
class MyFunctor {
public:
    int operator()(int x) const {
        return x * 2;
    }
};

MyFunctor func;
int result = func(5);  // Returns 10
```

## Why Use Functors?

Functors offer several advantages over regular functions:

1. **State**: Functors can maintain internal state between calls
2. **Configuration**: They can be configured with member variables
3. **Type-safe**: Strong typing at compile time
4. **Composable**: Easy to chain and combine operations
5. **Flexible**: Can implement multiple overloads of `operator()`

## Functor Patterns in This Example

### 1. Simple Stateless Functor

```cpp
class Squarer {
public:
    double operator()(double x) const {
        return x * x;
    }
};
```

**JavaScript Usage:**
```javascript
const squarer = new addon.Squarer();
console.log(squarer.call(5));  // 25

// Use with array methods
const squares = [1, 2, 3].map(x => squarer.call(x));
```

### 2. Stateful Functor

```cpp
class Accumulator {
private:
    double sum_;
    int count_;
public:
    double operator()(double value) {
        sum_ += value;
        count_++;
        return sum_;
    }
    // ... getters and reset method
};
```

**JavaScript Usage:**
```javascript
const acc = new addon.Accumulator();
console.log(acc.call(5));   // 5
console.log(acc.call(10));  // 15
console.log(acc.call(15));  // 30
console.log(acc.get_average());  // 10
```

### 3. Configurable Transform Functor

```cpp
class VectorTransform {
private:
    double scale_;
    double offset_;
public:
    std::vector<double> operator()(const std::vector<double>& input) const {
        std::vector<double> result;
        for (double val : input) {
            result.push_back(val * scale_ + offset_);
        }
        return result;
    }
    // ... getters and setters
};
```

**JavaScript Usage:**
```javascript
const transform = new addon.VectorTransform();
transform.set_scale(2.0);
transform.set_offset(3.0);

const output = transform.call([1, 2, 3, 4, 5]);
console.log(output);  // [5, 7, 9, 11, 13]
```

### 4. Predicate Functor

```cpp
class RangePredicate {
private:
    double min_, max_;
public:
    bool operator()(double value) const {
        return value >= min_ && value <= max_;
    }
    
    std::vector<double> filter(const std::vector<double>& input) const {
        std::vector<double> result;
        for (double val : input) {
            if ((*this)(val)) {
                result.push_back(val);
            }
        }
        return result;
    }
};
```

**JavaScript Usage:**
```javascript
const predicate = new addon.RangePredicate();
predicate.set_min(10);
predicate.set_max(50);

// Test individual values
console.log(predicate.test(25));  // true
console.log(predicate.test(75));  // false

// Filter array
const filtered = predicate.filter([5, 15, 25, 75, 35]);
console.log(filtered);  // [15, 25, 35]

// Use with JavaScript methods
const data = [5, 15, 25, 75, 35];
const jsFiltered = data.filter(x => predicate.test(x));
```

### 5. Binary Operation Functor

```cpp
class BinaryOp {
public:
    enum OpType { Add, Subtract, Multiply, Divide, Power };
private:
    OpType op_;
public:
    double operator()(double a, double b) const {
        switch (op_) {
            case Add: return a + b;
            case Multiply: return a * b;
            // ...
        }
    }
};
```

**JavaScript Usage:**
```javascript
const op = new addon.BinaryOp();

op.set_operation(0);  // Add
console.log(op.call(10, 5));  // 15

op.set_operation(2);  // Multiply
console.log(op.call(10, 5));  // 50

// Element-wise operations
const result = op.apply([1, 2, 3], [10, 20, 30]);
console.log(result);  // [10, 40, 90]
```

## How to Bind Functors

### Step 1: Define Your Functor Class

```cpp
class MyFunctor {
private:
    double factor_;
public:
    MyFunctor() : factor_(1.0) {}
    
    // The callable operator
    double operator()(double x) const {
        return x * factor_;
    }
    
    // Configuration methods
    void set_factor(double f) { factor_ = f; }
    double get_factor() const { return factor_; }
};
```

### Step 2: Register with Rosetta

```cpp
void register_functors() {
    ROSETTA_REGISTER_CLASS(MyFunctor)
        // Bind the operator() as "call" method
        .method("call", static_cast<double(MyFunctor::*)(double) const>(&MyFunctor::operator()))
        // Bind configuration methods
        .method("set_factor", &MyFunctor::set_factor)
        .field("factor", &MyFunctor::get_factor);
}
```

**Important:** The `static_cast` is necessary when binding `operator()` because it ensures the correct overload is selected, especially for const vs non-const versions.

### Step 3: Bind to JavaScript

```cpp
BEGIN_JS_MODULE(gen) {
    register_functors();
    gen.bind_class<MyFunctor>();
}
END_JS_MODULE();
```

### Step 4: Use in JavaScript

```javascript
const functor = new addon.MyFunctor();
functor.set_factor(3.0);
console.log(functor.call(5));  // 15
```

## Common Patterns

### Pattern 1: Pure Functions (Stateless)

Use when you need a reusable operation without state:

```cpp
class Doubler {
public:
    int operator()(int x) const { return x * 2; }
};
```

### Pattern 2: Accumulators (Stateful)

Use for running calculations, statistics, or aggregations:

```cpp
class Accumulator {
    double sum_ = 0;
public:
    double operator()(double x) { return sum_ += x; }
    double get_sum() const { return sum_; }
    void reset() { sum_ = 0; }
};
```

### Pattern 3: Configurable Operations

Use when you need parameterized behavior:

```cpp
class ScaledOperation {
    double scale_;
public:
    void set_scale(double s) { scale_ = s; }
    double operator()(double x) const { return x * scale_; }
};
```

### Pattern 4: Predicates (Boolean Tests)

Use for filtering, validation, and conditions:

```cpp
class ThresholdTest {
    double threshold_;
public:
    bool operator()(double x) const { return x > threshold_; }
};
```

### Pattern 5: Transformations

Use for data conversions and mappings:

```cpp
class Normalizer {
public:
    std::vector<double> operator()(const std::vector<double>& v) const {
        // Normalize to [0, 1] range
    }
};
```

## Real-World Example: Data Processing Pipeline

```javascript
// Load sensor data
const sensors = [95, 102, 300, 98, 101, -20, 99, 103];

// Step 1: Filter outliers
const validator = new addon.RangePredicate();
validator.set_min(90);
validator.set_max(110);
const valid = validator.filter(sensors);

// Step 2: Normalize data
const normalizer = new addon.VectorTransform();
normalizer.set_scale(1.0 / 100.0);
const normalized = normalizer.call(valid);

// Step 3: Calculate statistics
const stats = new addon.Accumulator();
valid.forEach(val => stats.call(val));
console.log('Average:', stats.get_average());
```

## Advantages of This Approach

1. **Type Safety**: Full C++ type checking
2. **Performance**: Native code execution
3. **Flexibility**: Easy to compose and chain operations
4. **Maintainability**: Clear separation of concerns
5. **Testability**: Easy to test both C++ and JavaScript sides
6. **Reusability**: Same functor can be used in multiple contexts

## Building the Example

```bash
# Create binding.gyp for Node.js addon
npm install
node-gyp configure
node-gyp build

# Run tests
node test_functors.js
node simple_functor_example.js
```

## Tips and Best Practices

1. **Always bind operator() as "call"**: This makes it intuitive in JavaScript
2. **Provide reset methods for stateful functors**: Allows reuse
3. **Include getters for configuration**: Enable inspection of functor state
4. **Use const for pure functors**: Indicates no side effects
5. **Document expected ranges and behavior**: Helps JavaScript users
6. **Consider thread safety**: If functors will be used concurrently
7. **Provide both single-value and vector methods**: Increases flexibility

## Comparison: Functions vs Functors

| Feature | Regular Function | Functor |
|---------|-----------------|---------|
| State | ❌ No | ✅ Yes |
| Configuration | ❌ No | ✅ Yes |
| Overloading | ✅ Yes | ✅ Yes |
| Composition | ⚠️ Limited | ✅ Easy |
| Polymorphism | ❌ No | ✅ Yes |
| Object-Oriented | ❌ No | ✅ Yes |

## Troubleshooting

### Problem: "error: no matching member function for call to 'method'"

**Solution**: Use `static_cast` to specify the exact signature:
```cpp
.method("call", static_cast<double(MyClass::*)(double) const>(&MyClass::operator()))
```

### Problem: Functor state not persisting

**Solution**: Make sure operator() is not const if you need to modify state:
```cpp
double operator()(double x) {  // Not const!
    sum_ += x;
    return sum_;
}
```

### Problem: Vector conversion fails

**Solution**: Register the vector converter:
```cpp
register_vector_converter<double>(gen);
```

## See Also

- Main binding example: `binding.cxx`
- Type converters: `type_converters.h`
- Rosetta documentation: `rosetta.h`