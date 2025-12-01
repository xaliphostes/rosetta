#pragma once

// Platform-specific export/import macros for dynamic library
#if defined(_WIN32) || defined(_WIN64)
    #ifdef FLOWER_EXPORTS
        #define FLOWER_API __declspec(dllexport)
    #else
        #define FLOWER_API __declspec(dllimport)
    #endif
#else
    // On Unix-like systems, use visibility attribute for better symbol handling
    #if __GNUC__ >= 4
        #define FLOWER_API __attribute__((visibility("default")))
    #else
        #define FLOWER_API
    #endif
#endif

class FLOWER_API Flower {
public:
    /**
     * @brief Computes the area of a circle given its radius
     * @param radius The radius of the circle
     * @return The area of the circle (π * r²)
     */
    double computeCircleArea(double radius);

    /**
     * @brief Computes the volume of a sphere given its radius
     * @param radius The radius of the sphere
     * @return The volume of the sphere (4/3 * π * r³)
     */
    double computeSphereVolume(double radius);

    /**
     * @brief Computes the Fibonacci number at position n
     * @param n The position in the Fibonacci sequence (0-indexed)
     * @return The nth Fibonacci number
     */
    long long computeFibonacci(int n);
};
