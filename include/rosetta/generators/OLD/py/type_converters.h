// ============================================================================
// Pre-built type converters for common C++ types using pybind11
// ============================================================================
#pragma once

#include "py_generator.h"
#include <array>
#include <map>
#include <memory>
#include <optional>
#include <pybind11/stl.h>
#include <unordered_map>
#include <vector>

namespace rosetta::generators::python {

    /**
     * @brief Register converters for std::vector<T>
     */
    template <typename T> inline void register_vector_converter(PyGenerator &gen) {
        // Register type in TypeRegistry
        TypeRegistry::instance().register_type<std::vector<T>>();

        // C++ vector<T> -> Python list
        gen.register_converter<std::vector<T>>(
            [](const rosetta::core::Any &val) -> py::object {
                try {
                    const auto &vec = val.as<std::vector<T>>();
                    return py::cast(vec);
                } catch (const std::bad_cast &e) {
                    std::cerr << "[ERROR] vector converter: bad_cast - " << e.what() << std::endl;
                    return py::none();
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] vector converter: " << e.what() << std::endl;
                    return py::none();
                }
            },
            // Python list -> C++ vector<T>
            [](const py::object &val) -> rosetta::core::Any {
                if (!PyList_Check(val.ptr()) && !PyTuple_Check(val.ptr())) {
                    return rosetta::core::Any();
                }

                try {
                    auto vec = val.cast<std::vector<T>>();
                    return rosetta::core::Any(vec);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] vector from Python: " << e.what() << std::endl;
                    return rosetta::core::Any();
                }
            });
    }

    /**
     * @brief Register converters for std::array<T, N>
     */
    template <typename T, size_t N> inline void register_array_converter(PyGenerator &gen) {
        gen.register_converter<std::array<T, N>>(
            [](const core::Any &val) -> py::object {
                try {
                    const auto &arr_cpp = val.as<std::array<T, N>>();
                    py::list    arr_py;
                    for (size_t i = 0; i < N; ++i) {
                        arr_py.append(arr_cpp[i]);
                    }
                    return arr_py;
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] array converter: " << e.what() << std::endl;
                    return py::none();
                }
            },
            [](const py::object &val) -> core::Any {
                if (!PyList_Check(val.ptr()) && !PyTuple_Check(val.ptr())) {
                    return core::Any();
                }

                try {
                    py::list         lst = val.cast<py::list>();
                    std::array<T, N> arr_cpp;
                    size_t           count = std::min(static_cast<size_t>(lst.size()), N);
                    for (size_t i = 0; i < count; ++i) {
                        arr_cpp[i] = lst[i].cast<T>();
                    }
                    return core::Any(arr_cpp);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] array from Python: " << e.what() << std::endl;
                    return core::Any();
                }
            });
    }

    /**
     * @brief Register converters for std::map<K, V>
     */
    template <typename K, typename V> inline void register_map_converter(PyGenerator &gen) {
        gen.register_converter<std::map<K, V>>(
            [](const core::Any &val) -> py::object {
                try {
                    const auto &map = val.as<std::map<K, V>>();
                    return py::cast(map);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] map converter: " << e.what() << std::endl;
                    return py::none();
                }
            },
            [](const py::object &val) -> core::Any {
                if (!PyDict_Check(val.ptr())) {
                    return core::Any();
                }

                try {
                    auto map = val.cast<std::map<K, V>>();
                    return core::Any(map);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] map from Python: " << e.what() << std::endl;
                    return core::Any();
                }
            });
    }

    /**
     * @brief Register converters for std::unordered_map<K, V>
     */
    template <typename K, typename V>
    inline void register_unordered_map_converter(PyGenerator &gen) {
        gen.register_converter<std::unordered_map<K, V>>(
            [](const core::Any &val) -> py::object {
                try {
                    const auto &map = val.as<std::unordered_map<K, V>>();
                    return py::cast(map);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] unordered_map converter: " << e.what() << std::endl;
                    return py::none();
                }
            },
            [](const py::object &val) -> core::Any {
                if (!PyDict_Check(val.ptr())) {
                    return core::Any();
                }

                try {
                    auto map = val.cast<std::unordered_map<K, V>>();
                    return core::Any(map);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] unordered_map from Python: " << e.what() << std::endl;
                    return core::Any();
                }
            });
    }

    /**
     * @brief Register converters for std::optional<T>
     */
    template <typename T> inline void register_optional_converter(PyGenerator &gen) {
        gen.register_converter<std::optional<T>>(
            [](const core::Any &val) -> py::object {
                try {
                    const auto &opt = val.as<std::optional<T>>();
                    if (!opt.has_value()) {
                        return py::none();
                    }
                    return py::cast(*opt);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] optional converter: " << e.what() << std::endl;
                    return py::none();
                }
            },
            [](const py::object &val) -> core::Any {
                if (val.is_none()) {
                    return core::Any(std::optional<T>());
                }

                try {
                    return core::Any(std::optional<T>(val.cast<T>()));
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] optional from Python: " << e.what() << std::endl;
                    return core::Any(std::optional<T>());
                }
            });
    }

    /**
     * @brief Register converters for std::shared_ptr<T>
     */
    template <typename T> inline void register_shared_ptr_converter(PyGenerator &gen) {
        gen.register_converter<std::shared_ptr<T>>(
            [](const core::Any &val) -> py::object {
                try {
                    const auto &ptr = val.as<std::shared_ptr<T>>();
                    if (!ptr) {
                        return py::none();
                    }
                    // Use pybind11's automatic shared_ptr handling
                    return py::cast(ptr);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] shared_ptr converter: " << e.what() << std::endl;
                    return py::none();
                }
            },
            [](const py::object &val) -> core::Any {
                if (val.is_none()) {
                    return core::Any(std::shared_ptr<T>());
                }

                try {
                    auto ptr = val.cast<std::shared_ptr<T>>();
                    return core::Any(ptr);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] shared_ptr from Python: " << e.what() << std::endl;
                    return core::Any(std::shared_ptr<T>());
                }
            });
    }

    /**
     * @brief Register converters for std::unique_ptr<T>
     * Note: Python will receive a raw pointer, not ownership
     */
    template <typename T> inline void register_unique_ptr_converter(PyGenerator &gen) {
        gen.register_converter<std::unique_ptr<T>>(
            [](const core::Any &val) -> py::object {
                try {
                    const auto &ptr = val.as<std::unique_ptr<T>>();
                    if (!ptr) {
                        return py::none();
                    }
                    // Cannot transfer ownership to Python, return raw pointer
                    return py::cast(ptr.get(), py::return_value_policy::reference);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] unique_ptr converter: " << e.what() << std::endl;
                    return py::none();
                }
            },
            [](const py::object &val) -> core::Any {
                // Cannot create unique_ptr from Python easily
                // Return empty unique_ptr
                return core::Any(std::unique_ptr<T>());
            });
    }

    /**
     * @brief Register converters for std::set<T>
     */
    template <typename T> inline void register_set_converter(PyGenerator &gen) {
        gen.register_converter<std::set<T>>(
            [](const core::Any &val) -> py::object {
                try {
                    const auto &set_cpp = val.as<std::set<T>>();
                    return py::cast(set_cpp);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] set converter: " << e.what() << std::endl;
                    return py::none();
                }
            },
            [](const py::object &val) -> core::Any {
                if (!PySet_Check(val.ptr()) && !PyList_Check(val.ptr())) {
                    return core::Any();
                }

                try {
                    auto set_cpp = val.cast<std::set<T>>();
                    return core::Any(set_cpp);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] set from Python: " << e.what() << std::endl;
                    return core::Any();
                }
            });
    }

    /**
     * @brief Register converters for std::pair<T1, T2>
     */
    template <typename T1, typename T2> inline void register_pair_converter(PyGenerator &gen) {
        gen.register_converter<std::pair<T1, T2>>(
            [](const core::Any &val) -> py::object {
                try {
                    const auto &pair = val.as<std::pair<T1, T2>>();
                    return py::cast(pair);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] pair converter: " << e.what() << std::endl;
                    return py::none();
                }
            },
            [](const py::object &val) -> core::Any {
                if (!PyTuple_Check(val.ptr())) {
                    return core::Any();
                }

                try {
                    auto pair = val.cast<std::pair<T1, T2>>();
                    return core::Any(pair);
                } catch (const std::exception &e) {
                    std::cerr << "[ERROR] pair from Python: " << e.what() << std::endl;
                    return core::Any();
                }
            });
    }

    /**
     * @brief Convenience function to register all common converters
     */
    inline void register_common_converters(PyGenerator &gen) {
        // Basic vectors
        register_vector_converter<int>(gen);
        register_vector_converter<long>(gen);
        register_vector_converter<double>(gen);
        register_vector_converter<float>(gen);
        register_vector_converter<std::string>(gen);
        register_vector_converter<bool>(gen);

        // Common arrays
        register_array_converter<double, 2>(gen);
        register_array_converter<double, 3>(gen);
        register_array_converter<double, 4>(gen);
        register_array_converter<float, 3>(gen);
        register_array_converter<int, 3>(gen);

        // Maps
        register_map_converter<std::string, int>(gen);
        register_map_converter<std::string, double>(gen);
        register_map_converter<std::string, std::string>(gen);
        register_map_converter<int, int>(gen);
        register_map_converter<int, double>(gen);

        // Unordered maps
        register_unordered_map_converter<std::string, int>(gen);
        register_unordered_map_converter<std::string, double>(gen);
        register_unordered_map_converter<std::string, std::string>(gen);

        // Optionals
        register_optional_converter<int>(gen);
        register_optional_converter<double>(gen);
        register_optional_converter<float>(gen);
        register_optional_converter<std::string>(gen);
        register_optional_converter<bool>(gen);

        // Sets
        register_set_converter<int>(gen);
        register_set_converter<std::string>(gen);
        register_set_converter<double>(gen);

        // Pairs
        register_pair_converter<int, int>(gen);
        register_pair_converter<std::string, int>(gen);
        register_pair_converter<std::string, std::string>(gen);
    }

    /**
     * @brief Helper to register NumPy array converters (if NumPy is available)
     */
    inline void register_numpy_converters(PyGenerator &gen) {
        // This would require pybind11/numpy.h
        // Implementation depends on whether NumPy support is enabled

        // Example for vector<double> -> numpy array:
        /*
        gen.register_converter<std::vector<double>>(
            [](const core::Any &val) -> py::object {
                const auto &vec = val.as<std::vector<double>>();
                return py::array_t<double>(vec.size(), vec.data());
            },
            [](const py::object &val) -> core::Any {
                auto arr = val.cast<py::array_t<double>>();
                std::vector<double> vec(arr.size());
                std::memcpy(vec.data(), arr.data(), arr.size() * sizeof(double));
                return core::Any(vec);
            }
        );
        */
    }

} // namespace rosetta::generators::python