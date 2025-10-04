#pragma once
#include <rosetta/generators/details/js/js_generator.h>
#include <rosetta/function_registry.h>

namespace rosetta {

    /**
     * @brief Bind a single function by name
     */
    void bindFunction(JsGenerator &generator, const std::string &func_name);

    /**
     * @brief Bind multiple specific functions by name
     */
    inline void bindFunctions(JsGenerator &generator, const std::vector<std::string> &func_names);

    /**
     * @brief Bind ALL registered functions
     */
    inline void bindAllFunctions(JsGenerator &generator);

} // namespace rosetta

#include "inline/js_functions.hxx"
