#include <functional>

// ============================================================================
// Automatic Function Converter Registration
// ============================================================================

namespace rosetta::core {

    // ============================================================================
    // Template metaprogramming to extract function types from method signatures
    // ============================================================================

    /**
     * @brief Helper to extract std::function types from method parameters
     */
    template <typename T> struct function_param_extractor {
        static constexpr bool is_function = false;
    };

    // Specialization for std::function
    template <typename Ret, typename... Args>
    struct function_param_extractor<std::function<Ret(Args...)>> {
        static constexpr bool is_function = true;
        using return_type                 = Ret;
        using arg_tuple                   = std::tuple<Args...>;

        static void register_converter() {
            rosetta::core::register_function_converter<Ret, Args...>();
        }
    };

    // Also handle const references to std::function
    template <typename Ret, typename... Args>
    struct function_param_extractor<const std::function<Ret(Args...)> &> {
        static constexpr bool is_function = true;
        using return_type                 = Ret;
        using arg_tuple                   = std::tuple<Args...>;

        static void register_converter() {
            rosetta::core::register_function_converter<Ret, Args...>();
        }
    };

    /**
     * @brief Extract and register function converters from method parameters
     */
    template <typename... Args> void register_function_converters_for_params() {
        // Use fold expression to register converters for each parameter
        (
            []() {
                using CleanType = std::remove_cv_t<std::remove_reference_t<Args>>;
                if constexpr (function_param_extractor<CleanType>::is_function) {
                    function_param_extractor<CleanType>::register_converter();
                }
            }(),
            ...);
    }

    /**
     * @brief Auto-detect and register function converters from a method signature
     *
     * This version works for non-const methods
     */
    template <typename Class, typename Ret, typename... Args>
    void auto_register_function_converters(Ret (Class::*)(Args...)) {
        register_function_converters_for_params<Args...>();
    }

    /**
     * @brief Auto-detect and register function converters from a const method signature
     */
    template <typename Class, typename Ret, typename... Args>
    void auto_register_function_converters(Ret (Class::*)(Args...) const) {
        register_function_converters_for_params<Args...>();
    }

} // namespace rosetta::core
