/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 */
#include <functional>
#include <memory>
#include "../js_generator.h"
#include "../js_common.h"

namespace rosetta {

    /**
     * @brief Base class for wrapping C++ functors/lambdas
     */
    class FunctorBase {
    public:
        virtual ~FunctorBase() = default;
        virtual Napi::Value call(Napi::Env env, const std::vector<Napi::Value>& args) = 0;
    };

    /**
     * @brief Template wrapper for specific functor types
     */
    template <typename Ret, typename... Args> class FunctorWrapper : public FunctorBase {
    public:
        using FunctionType = std::function<Ret(Args...)>;

        explicit FunctorWrapper(FunctionType func)
            : func_(std::move(func))
        {
        }

        Napi::Value call(Napi::Env env, const std::vector<Napi::Value>& args) override
        {
            // Validate argument count
            if (args.size() != sizeof...(Args)) {
                Napi::TypeError::New(env,
                    "Expected " + std::to_string(sizeof...(Args)) + " arguments, got "
                        + std::to_string(args.size()))
                    .ThrowAsJavaScriptException();
                return env.Undefined();
            }

            // Call the function with converted arguments
            return callImpl(env, args, std::index_sequence_for<Args...> {});
        }

    private:
        FunctionType func_;

        template <std::size_t... I>
        Napi::Value callImpl(
            Napi::Env env, const std::vector<Napi::Value>& args, std::index_sequence<I...>)
        {
            if constexpr (std::is_void_v<Ret>) {
                func_(fromNapiValue<Args>(args[I])...);
                return env.Undefined();
            } else {
                auto result = func_(fromNapiValue<Args>(args[I])...);
                return toNapiValue(env, result);
            }
        }
    };

    // ============================================================================
    // JavaScript function wrapper class
    // ============================================================================

    class JsFunction : public Napi::ObjectWrap<JsFunction> {
    public:
        static Napi::Object Init(Napi::Env env, Napi::Object exports);
        static Napi::Object NewInstance(Napi::Env env, std::shared_ptr<FunctorBase> functor);

        JsFunction(const Napi::CallbackInfo& info);

    private:
        std::shared_ptr<FunctorBase> functor_;

        Napi::Value Call(const Napi::CallbackInfo& info);
    };

    // ============================================================================

    template <typename Ret, typename... Args>
    inline Napi::Value functorToJs(Napi::Env env, std::function<Ret(Args...)> func)
    {
        auto wrapper = std::make_shared<FunctorWrapper<Ret, Args...>>(std::move(func));
        return JsFunction::NewInstance(env, wrapper);
    }

    template <typename Lambda> inline Napi::Value lambdaToJs(Napi::Env env, Lambda&& lambda)
    {
        // using FuncType = decltype(std::function { std::forward<Lambda>(lambda) });
        return functorToJs(env, std::function { std::forward<Lambda>(lambda) });
    }

    /**
     * @brief Helper to wrap JavaScript function with proper lifetime management
     */
    class JsFunctionWrapper {
    public:
        explicit JsFunctionWrapper(const Napi::Function& func);

        template <typename Ret, typename... Args> Ret call(Args... args) const;

    private:
        Napi::FunctionReference func_ref_;
        Napi::Env env_;
    };

    // ============================================================================
    // JS to C++ functor conversion
    // ============================================================================

    /**
     * @brief Convert JavaScript function to C++ std::function
     * @tparam Ret Return type
     * @tparam Args Argument types
     * @param js_func JavaScript function
     * @return C++ std::function that calls the JavaScript function
     */
    template <typename Ret, typename... Args> inline std::any jsToFunctor(const Napi::Value& js_val)
    {
        if (!js_val.IsFunction()) {
            throw std::runtime_error("Expected JavaScript function");
        }

        auto js_func = js_val.As<Napi::Function>();
        auto wrapper = std::make_shared<JsFunctionWrapper>(js_func);

        // Create C++ std::function that calls JavaScript function
        std::function<Ret(Args...)> cpp_func = [wrapper](Args... args) -> Ret {
            return wrapper->call<Ret, Args...>(std::forward<Args>(args)...);
        };

        return cpp_func;
    }

    template <typename Ret, typename... Args>
    inline void registerFunctorType(JsGenerator& generator)
    {
        using FuncType = std::function<Ret(Args...)>;
        std::string typeName = typeid(FuncType).name();

        generator.register_type_converter(
            typeName,
            // C++ to JS: Wrap C++ lambda as JavaScript function
            [](Napi::Env env, const std::any& value) -> Napi::Value {
                try {
                    auto func = std::any_cast<FuncType>(value);
                    return functorToJs(env, func);
                } catch (const std::bad_any_cast&) {
                    return env.Undefined();
                }
            },
            // JS to C++: Wrap JavaScript function as C++ std::function
            [](const Napi::Value& js_val) -> std::any {
                return jsToFunctor<Ret, Args...>(js_val);
            });
    }

    // ============================================================================
    // JsFunction implementation
    // ============================================================================

    inline Napi::Object JsFunction::Init(Napi::Env env, Napi::Object exports)
    {
        Napi::Function func
            = DefineClass(env, "CppFunction", { InstanceMethod("call", &JsFunction::Call) });

        // Store constructor for later use
        Napi::FunctionReference* constructor = new Napi::FunctionReference();
        *constructor = Napi::Persistent(func);
        env.SetInstanceData(constructor);

        return exports;
    }

    inline Napi::Object JsFunction::NewInstance(Napi::Env env, std::shared_ptr<FunctorBase> functor)
    {
        // Create a callable JavaScript function
        auto jsFunc
            = Napi::Function::New(env, [functor](const Napi::CallbackInfo& info) -> Napi::Value {
                  std::vector<Napi::Value> args;
                  for (size_t i = 0; i < info.Length(); ++i) {
                      args.push_back(info[i]);
                  }
                  return functor->call(info.Env(), args);
              });

        return jsFunc;
    }

    inline JsFunction::JsFunction(const Napi::CallbackInfo& info)
        : Napi::ObjectWrap<JsFunction>(info)
    {
        // Functor will be set via NewInstance
    }

    inline Napi::Value JsFunction::Call(const Napi::CallbackInfo& info)
    {
        std::vector<Napi::Value> args;
        for (size_t i = 0; i < info.Length(); ++i) {
            args.push_back(info[i]);
        }
        return functor_->call(info.Env(), args);
    }

    // ============================================================================
    // JsFunctionWrapper implementation
    // ============================================================================

    inline JsFunctionWrapper::JsFunctionWrapper(const Napi::Function& func)
        : env_(func.Env())
    {
        func_ref_ = Napi::Persistent(func);
    }

    template <typename Ret, typename... Args> inline Ret JsFunctionWrapper::call(Args... args) const
    {
        // Convert C++ arguments to JavaScript values
        std::vector<napi_value> js_args;
        js_args.reserve(sizeof...(Args));

        // Fold expression to convert each argument
        ((js_args.push_back(toNapiValue(env_, args))), ...);

        // Call JavaScript function
        Napi::Value result = func_ref_.Call(js_args);

        // Convert result back to C++
        if constexpr (std::is_void_v<Ret>) {
            return;
        } else {
            return fromNapiValue<Ret>(result);
        }
    }

    // ============================================================================
    // Generator extension
    // ============================================================================

    inline void registerFunctorSupport(JsGenerator& generator)
    {
        // Register common functor types (bidirectional)

        // Unary functors
        registerFunctorType<void, int>(generator);
        registerFunctorType<void, double>(generator);
        registerFunctorType<void, const std::string&>(generator);
        registerFunctorType<int, int>(generator);
        registerFunctorType<double, double>(generator);
        registerFunctorType<std::string, const std::string&>(generator);

        // Binary functors (for reduce)
        registerFunctorType<int, int, int>(generator);
        registerFunctorType<double, double, double>(generator);
        registerFunctorType<std::string, const std::string&, const std::string&>(generator);

        // Predicates
        registerFunctorType<bool, int>(generator);
        registerFunctorType<bool, double>(generator);
        registerFunctorType<bool, const std::string&>(generator);

        // Index-based functors (forEach with index)
        registerFunctorType<void, int, size_t>(generator);
        registerFunctorType<void, double, size_t>(generator);
        registerFunctorType<void, const std::string&, size_t>(generator);
    }

} // namespace rosetta