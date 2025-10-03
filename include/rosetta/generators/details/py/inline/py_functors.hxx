/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 */
#include <functional>
#include <memory>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <rosetta/generators/details/py/py_generator.h>

namespace py = pybind11;

namespace rosetta {

    // ============================================================================
    // C++ to Python functor conversion
    // ============================================================================

    /**
     * @brief Convert C++ std::function to Python callable
     * @tparam Ret Return type
     * @tparam Args Argument types
     */
    template <typename Ret, typename... Args>
    inline py::object functorToPython(const std::function<Ret(Args...)>& func)
    {
        // pybind11 has built-in support for std::function, but we wrap it
        // for consistency with our conversion system
        return py::cpp_function([func](Args... args) -> Ret { return func(args...); });
    }

    /**
     * @brief Convert C++ lambda to Python callable
     * @tparam Lambda Lambda type
     */
    template <typename Lambda> inline py::object lambdaToPython(Lambda&& lambda)
    {
        return py::cpp_function(std::forward<Lambda>(lambda));
    }

    // ============================================================================
    // Python to C++ functor conversion
    // ============================================================================

    /**
     * @brief Helper to wrap Python callable with proper lifetime management
     */
    class PyCallableWrapper {
    public:
        explicit PyCallableWrapper(py::object callable)
            : callable_(callable)
        {
        }

        template <typename Ret, typename... Args> Ret call(Args... args) const
        {
            py::gil_scoped_acquire gil;

            try {
                py::object result = callable_(args...);

                if constexpr (std::is_void_v<Ret>) {
                    return;
                } else {
                    return result.cast<Ret>();
                }
            } catch (const py::error_already_set& e) {
                throw std::runtime_error(std::string("Python callback error: ") + e.what());
            }
        }

    private:
        py::object callable_;
    };

    /**
     * @brief Convert Python callable to C++ std::function
     * @tparam Ret Return type
     * @tparam Args Argument types
     * @param py_obj Python callable object
     * @return C++ std::function that calls the Python callable
     */
    template <typename Ret, typename... Args>
    inline std::any pythonToFunctor(const py::object& py_obj)
    {
        if (!py::isinstance<py::function>(py_obj) && !py::hasattr(py_obj, "__call__")) {
            throw std::runtime_error("Expected Python callable");
        }

        auto wrapper = std::make_shared<PyCallableWrapper>(py_obj);

        // Create C++ std::function that calls Python callable
        std::function<Ret(Args...)> cpp_func = [wrapper](Args... args) -> Ret {
            return wrapper->call<Ret, Args...>(std::forward<Args>(args)...);
        };

        return cpp_func;
    }

    // ============================================================================
    // Type registration
    // ============================================================================

    /**
     * @brief Register functor type converter (bidirectional)
     * @tparam Ret Return type
     * @tparam Args Argument types
     */
    template <typename Ret, typename... Args>
    inline void registerFunctorType(PyGenerator& generator)
    {
        // Note: pybind11 has automatic support for std::function via pybind11/functional.h
        // This registration is for explicit control and consistency with the JS/Lua systems

        // The conversion is handled automatically by pybind11's built-in converters
        // when pybind11/functional.h is included

        // This function exists for API consistency but doesn't need explicit registration
        // in Python since pybind11 handles it automatically
    }

    /**
     * @brief Register common functor types
     */
    inline void registerFunctorSupport(PyGenerator& generator)
    {
        // With pybind11, functor support is built-in via pybind11/functional.h
        // Common functor types are automatically handled:

        // Unary functors
        // - void(int), void(double), void(const std::string&)
        // - int(int), double(double), std::string(const std::string&)

        // Binary functors (for reduce)
        // - int(int, int), double(double, double)
        // - std::string(const std::string&, const std::string&)

        // Predicates
        // - bool(int), bool(double), bool(const std::string&)

        // Index-based functors (forEach with index)
        // - void(int, size_t), void(double, size_t)
        // - void(const std::string&, size_t)

        // No explicit registration needed - pybind11 handles this automatically
        // This function exists for API consistency with JS and Lua generators
    }

} // namespace rosetta

// ============================================================================
// USAGE GUIDE FOR PYTHON
// ============================================================================
/*

BASIC USAGE:
-----------

    #include <rosetta/generators/py.h>
    #include <rosetta/generators/py_functors.h>
    #include <pybind11/functional.h>

    class DataProcessor : public rosetta::Introspectable {
        INTROSPECTABLE(DataProcessor)
    public:
        DataProcessor(const std::vector<double>& data) : data_(data) {}

        // C++ → Python: Return a lambda/functor
        std::function<double(double)> multiplier(double factor) const {
            return [factor](double x) { return x * factor; };
        }

        // Python → C++: Accept Python callable
        std::vector<double> filter(std::function<bool(double)> predicate) const {
            std::vector<double> result;
            std::copy_if(data_.begin(), data_.end(),
                        std::back_inserter(result), predicate);
            return result;
        }

        void forEach(std::function<void(double)> callback) const {
            std::for_each(data_.begin(), data_.end(), callback);
        }

    private:
        std::vector<double> data_;
    };

    // Binding
    PYBIND11_MODULE(dataproc, m) {
        rosetta::PyGenerator generator(m);

        // Functor support is automatic with pybind11/functional.h
        // No explicit registration needed!

        generator.bind_class<DataProcessor>();
    }


PYTHON USAGE:
------------

    import dataproc

    proc = dataproc.DataProcessor([1, 2, 3, 4, 5])

    # C++ → Python: Get a function from C++
    mult = proc.multiplier(10)
    print(mult(3))  # 30

    # Python → C++: Pass Python lambda to C++
    filtered = proc.filter(lambda x: x > 3)
    print(filtered)  # [4, 5]

    # Python → C++: Pass Python function to C++
    proc.forEach(lambda x: print(f"Element: {x}"))


ADVANCED USAGE:
--------------

    class Calculator : public rosetta::Introspectable {
        INTROSPECTABLE(Calculator)
    public:
        // Binary operations
        double reduce(std::function<double(double, double)> op,
                     const std::vector<double>& values) const {
            return std::accumulate(values.begin() + 1, values.end(),
                                  values[0], op);
        }

        // Transform with index
        std::vector<double> mapIndexed(
            std::function<double(double, size_t)> transform,
            const std::vector<double>& values) const {
            std::vector<double> result;
            for (size_t i = 0; i < values.size(); ++i) {
                result.push_back(transform(values[i], i));
            }
            return result;
        }

        // Higher-order function returning function
        std::function<double(double)> compose(
            std::function<double(double)> f,
            std::function<double(double)> g) const {
            return [f, g](double x) { return f(g(x)); };
        }
    };

Python usage:
    calc = Calculator()

    # Reduce
    sum_val = calc.reduce(lambda a, b: a + b, [1, 2, 3, 4, 5])
    print(sum_val)  # 15

    # Map with index
    indexed = calc.mapIndexed(lambda x, i: x * i, [10, 20, 30])
    print(indexed)  # [0, 20, 60]

    # Function composition
    double = lambda x: x * 2
    square = lambda x: x * x
    composed = calc.compose(double, square)
    print(composed(3))  # double(square(3)) = double(9) = 18


COMPARISON: Python vs JavaScript vs Lua
---------------------------------------

Python (pybind11):
    ✓ Automatic functor conversion (via pybind11/functional.h)
    ✓ No explicit registration needed
    ✓ GIL handled automatically
    ✓ Type conversions work seamlessly
    ✓ Exception propagation works correctly

JavaScript (N-API):
    ✗ Requires explicit converter registration
    ✗ Manual lifetime management needed
    ✗ Custom wrapper classes required
    ✗ More complex implementation

Lua (Sol3):
    ~ Semi-automatic conversion
    ~ Requires some registration
    ~ Simpler than JavaScript
    ~ More complex than Python


KEY FEATURES:
------------

1. Bidirectional Conversion:
   - C++ std::function → Python callable
   - Python callable → C++ std::function

2. Automatic Type Conversion:
   - Arguments converted automatically
   - Return values converted automatically
   - Works with all pybind11-supported types

3. Exception Handling:
   - Python exceptions caught and converted to C++
   - C++ exceptions propagated to Python

4. Lifetime Management:
   - Python GIL acquired when needed
   - Objects kept alive automatically
   - No manual reference counting


SUPPORTED SIGNATURES:
--------------------

Unary functions:
    void(int), void(double), void(std::string)
    int(int), double(double), std::string(std::string)

Binary functions:
    int(int, int), double(double, double)
    std::string(std::string, std::string)

Predicates:
    bool(int), bool(double), bool(std::string)

Index-based:
    void(T, size_t), T(T, size_t)

Custom types:
    Any type supported by pybind11


ERROR HANDLING:
--------------

Python callable errors are caught and converted:

    try:
        proc.filter(lambda x: x / 0)  # Division by zero
    except RuntimeError as e:
        print(e)  # "Python callback error: division by zero"


PERFORMANCE NOTES:
-----------------

1. GIL Overhead:
   - Python GIL acquired for each callback
   - Use C++ lambdas when possible for performance
   - Batch operations to minimize GIL acquisitions

2. Type Conversion:
   - Conversion overhead for arguments/returns
   - Negligible for primitive types
   - Consider for large objects

3. Best Practices:
   - Keep callbacks simple and fast
   - Avoid frequent small callbacks
   - Use C++ for performance-critical loops


INTEGRATION EXAMPLES:
--------------------

Numerical Processing:
    def integrate(func, a, b, n=1000):
        return calc.integrate(func, a, b, n)

    result = integrate(lambda x: x**2, 0, 1)

Data Filtering:
    users = [User(...), User(...), ...]
    adults = filter_users(lambda u: u.age >= 18, users)

Event Callbacks:
    button.on_click(lambda: print("Clicked!"))

Async Operations:
    async def callback(data):
        await process(data)

    processor.set_callback(callback)


DEBUGGING:
---------

Check if callable:
    import inspect
    print(callable(proc.multiplier(10)))  # True
    print(inspect.isfunction(proc.multiplier(10)))

Type inspection:
    import typing
    func = proc.multiplier(10)
    print(type(func))  # <class 'function'>

Error tracing:
    import traceback
    try:
        proc.filter(lambda x: 1/0)
    except Exception:
        traceback.print_exc()


COMPARISON WITH MANUAL BINDING:
-------------------------------

Manual pybind11 (complex):
    m.def("filter", [](DataProcessor& self, py::function pred) {
        // Manual conversion
        std::function<bool(double)> cpp_pred =
            [pred](double x) {
                py::gil_scoped_acquire gil;
                return pred(x).cast<bool>();
            };
        return self.filter(cpp_pred);
    });

Automatic (simple):
    // Just bind the method - pybind11 handles the rest!
    .method("filter", &DataProcessor::filter)


LIMITATIONS:
-----------

1. Python GIL:
   - Must acquire GIL for Python callbacks
   - Can't use Python callbacks in C++ threads
   - Use py::call_guard<py::gil_scoped_release>() for long operations

2. Lifetime:
   - Python callables keep C++ objects alive
   - Be careful with circular references
   - Use weak references when needed

3. Performance:
   - Python callbacks slower than C++ lambdas
   - Not suitable for tight inner loops
   - Consider Cython/numba for performance-critical code

*/