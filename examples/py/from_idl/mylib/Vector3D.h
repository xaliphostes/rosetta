// ============================================================================
// 3D Vector implementation for the geometry example
// ============================================================================
#pragma once

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace mylib {

    /**
     * @brief 3D vector class
     *
     * Represents a 3-dimensional vector with x, y, z components.
     * Provides common vector operations like length, normalization,
     * dot product, and cross product.
     */
    class Vector3D {
    public:
        // ========================================================================
        // Public fields
        // ========================================================================

        double x; ///< X component
        double y; ///< Y component
        double z; ///< Z component

        // ========================================================================
        // Constructors
        // ========================================================================

        /**
         * @brief Default constructor - creates zero vector (0, 0, 0)
         */
        Vector3D() : x(0.0), y(0.0), z(0.0) {}

        /**
         * @brief Constructor with coordinates
         * @param x X component
         * @param y Y component
         * @param z Z component
         */
        Vector3D(double x, double y, double z) : x(x), y(y), z(z) {}

        /**
         * @brief Copy constructor
         */
        Vector3D(const Vector3D &other) = default;

        /**
         * @brief Move constructor
         */
        Vector3D(Vector3D &&other) noexcept = default;

        // ========================================================================
        // Assignment operators
        // ========================================================================

        Vector3D &operator=(const Vector3D &other)     = default;
        Vector3D &operator=(Vector3D &&other) noexcept = default;

        // ========================================================================
        // Vector operations
        // ========================================================================

        /**
         * @brief Calculate the length (magnitude) of the vector
         * @return Length of the vector
         */
        double length() const { return std::sqrt(x * x + y * y + z * z); }

        /**
         * @brief Calculate the squared length (avoids sqrt)
         * @return Squared length of the vector
         */
        double lengthSquared() const { return x * x + y * y + z * z; }

        /**
         * @brief Return a normalized version of this vector
         * @return Normalized vector (unit vector)
         * @note Returns zero vector if length is zero
         */
        Vector3D normalize() const {
            double len = length();
            if (len < 1e-10) {
                return Vector3D(0.0, 0.0, 0.0);
            }
            return Vector3D(x / len, y / len, z / len);
        }

        /**
         * @brief Normalize this vector in place
         */
        void normalizeInPlace() {
            double len = length();
            if (len >= 1e-10) {
                x /= len;
                y /= len;
                z /= len;
            }
        }

        /**
         * @brief Calculate dot product with another vector
         * @param other The other vector
         * @return Dot product (scalar)
         */
        double dot(const Vector3D &other) const { return x * other.x + y * other.y + z * other.z; }

        /**
         * @brief Calculate cross product with another vector
         * @param other The other vector
         * @return Cross product vector
         */
        Vector3D cross(const Vector3D &other) const {
            return Vector3D(y * other.z - z * other.y, z * other.x - x * other.z,
                            x * other.y - y * other.x);
        }

        /**
         * @brief Calculate distance to another vector
         * @param other The other vector
         * @return Distance between the two vectors
         */
        double distanceTo(const Vector3D &other) const {
            double dx = x - other.x;
            double dy = y - other.y;
            double dz = z - other.z;
            return std::sqrt(dx * dx + dy * dy + dz * dz);
        }

        /**
         * @brief Calculate squared distance to another vector
         * @param other The other vector
         * @return Squared distance between the two vectors
         */
        double distanceSquaredTo(const Vector3D &other) const {
            double dx = x - other.x;
            double dy = y - other.y;
            double dz = z - other.z;
            return dx * dx + dy * dy + dz * dz;
        }

        /**
         * @brief Linear interpolation between this vector and another
         * @param other The target vector
         * @param t Interpolation factor (0.0 to 1.0)
         * @return Interpolated vector
         */
        Vector3D lerp(const Vector3D &other, double t) const {
            return Vector3D(x + (other.x - x) * t, y + (other.y - y) * t, z + (other.z - z) * t);
        }

        /**
         * @brief Convert to array representation
         * @return Vector as std::vector<double> with 3 elements [x, y, z]
         */
        std::vector<double> to_array() const { return {x, y, z}; }

        /**
         * @brief Convert to string representation
         * @return String in format "(x, y, z)"
         */
        std::string toString() const {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3);
            oss << "(" << x << ", " << y << ", " << z << ")";
            return oss.str();
        }

        // ========================================================================
        // Arithmetic operators
        // ========================================================================

        Vector3D operator+(const Vector3D &other) const {
            return Vector3D(x + other.x, y + other.y, z + other.z);
        }

        Vector3D operator-(const Vector3D &other) const {
            return Vector3D(x - other.x, y - other.y, z - other.z);
        }

        Vector3D operator*(double scalar) const {
            return Vector3D(x * scalar, y * scalar, z * scalar);
        }

        Vector3D operator/(double scalar) const {
            return Vector3D(x / scalar, y / scalar, z / scalar);
        }

        Vector3D operator-() const { return Vector3D(-x, -y, -z); }

        Vector3D &operator+=(const Vector3D &other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        Vector3D &operator-=(const Vector3D &other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        Vector3D &operator*=(double scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }

        Vector3D &operator/=(double scalar) {
            x /= scalar;
            y /= scalar;
            z /= scalar;
            return *this;
        }

        // ========================================================================
        // Comparison operators
        // ========================================================================

        bool operator==(const Vector3D &other) const {
            const double epsilon = 1e-9;
            return std::abs(x - other.x) < epsilon && std::abs(y - other.y) < epsilon &&
                   std::abs(z - other.z) < epsilon;
        }

        bool operator!=(const Vector3D &other) const { return !(*this == other); }

        // ========================================================================
        // Static factory methods
        // ========================================================================

        /**
         * @brief Create zero vector (0, 0, 0)
         */
        static Vector3D zero() { return Vector3D(0.0, 0.0, 0.0); }

        /**
         * @brief Create unit X vector (1, 0, 0)
         */
        static Vector3D unitX() { return Vector3D(1.0, 0.0, 0.0); }

        /**
         * @brief Create unit Y vector (0, 1, 0)
         */
        static Vector3D unitY() { return Vector3D(0.0, 1.0, 0.0); }

        /**
         * @brief Create unit Z vector (0, 0, 1)
         */
        static Vector3D unitZ() { return Vector3D(0.0, 0.0, 1.0); }

        /**
         * @brief Create vector with all components set to the same value
         */
        static Vector3D uniform(double value) { return Vector3D(value, value, value); }
    };

    // ============================================================================
    // Free functions
    // ============================================================================

    /**
     * @brief Create a vector from coordinates
     */
    inline Vector3D create_vector(double x = 0.0, double y = 0.0, double z = 0.0) {
        return Vector3D(x, y, z);
    }

    /**
     * @brief Calculate distance between two vectors
     */
    inline double distance(const Vector3D &a, const Vector3D &b) {
        return a.distanceTo(b);
    }

    /**
     * @brief Linear interpolation between two vectors
     */
    inline Vector3D lerp(const Vector3D &a, const Vector3D &b, double t) {
        return a.lerp(b, t);
    }

    /**
     * @brief Scalar multiplication (scalar on left side)
     */
    inline Vector3D operator*(double scalar, const Vector3D &vec) {
        return vec * scalar;
    }

    /**
     * @brief Stream output operator
     */
    inline std::ostream &operator<<(std::ostream &os, const Vector3D &vec) {
        os << vec.toString();
        return os;
    }

} // namespace mylib
