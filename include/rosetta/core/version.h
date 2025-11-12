#pragma once
#include <iostream>
#include <string>

// Version
#define ROSETTA_VERSION_MAJOR 1
#define ROSETTA_VERSION_MINOR 0
#define ROSETTA_VERSION_PATCH 0

namespace rosetta::core {

    // ============================================================================
    // Version and Info
    // ============================================================================

    /**
     * @brief Get the Rosetta version as a string
     * @return String with format "major.minor.patch"
     */
    inline std::string version() {
        return std::to_string(ROSETTA_VERSION_MAJOR) + "." + std::to_string(ROSETTA_VERSION_MINOR) +
               "." + std::to_string(ROSETTA_VERSION_PATCH);
    }

    /**
     * @brief Display Rosetta information to stdout
     */
    inline void print_info() {
        std::cout << "Rosetta C++ Introspection Library\n";
        std::cout << "Version: " << version() << "\n";
        std::cout << "Features:\n";
        std::cout << "  - Non-intrusive class registration\n";
        std::cout << "  - Automatic binding generation (Python, JS, TS)\n";
        std::cout << "  - Serialization (JSON, XML)\n";
        std::cout << "  - Validation with constraints\n";
        std::cout << "  - Documentation generation\n";
    }

} // namespace rosetta::core