#pragma once
#include <string>

// ============================================================================
// Type Mapping for Multiple Targets
// ============================================================================
struct TypeInfo {
    std::string cpp_type;
    std::string python_type;
    std::string js_type;
    std::string ts_type;
    bool is_primitive;
    bool needs_conversion;
};
