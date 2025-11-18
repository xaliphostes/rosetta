// ============================================================================
// mylib/Matrix4x4.h
//
// 4x4 Matrix implementation for 3D transformations
// ============================================================================
#pragma once

#include "Vector3D.h"
#include <array>
#include <cmath>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

namespace mylib {

    /**
     * @brief 4x4 matrix for 3D transformations
     *
     * Stored in column-major order (compatible with OpenGL).
     * Layout:
     * [0  4  8  12]   [m00 m01 m02 m03]
     * [1  5  9  13] = [m10 m11 m12 m13]
     * [2  6  10 14]   [m20 m21 m22 m23]
     * [3  7  11 15]   [m30 m31 m32 m33]
     */
    class Matrix4x4 {
    private:
        std::array<double, 16> data_;

    public:
        // ========================================================================
        // Constructors
        // ========================================================================

        /**
         * @brief Default constructor - creates identity matrix
         */
        Matrix4x4() {
            data_ = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                     0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
        }

        /**
         * @brief Constructor from array of 16 values
         * @param values Array in column-major order
         */
        explicit Matrix4x4(const std::array<double, 16> &values) : data_(values) {}

        /**
         * @brief Constructor from individual elements (row-major for convenience)
         */
        Matrix4x4(double m00, double m01, double m02, double m03, double m10, double m11,
                  double m12, double m13, double m20, double m21, double m22, double m23,
                  double m30, double m31, double m32, double m33) {
            // Convert row-major to column-major
            data_ = {m00, m10, m20, m30, m01, m11, m21, m31,
                     m02, m12, m22, m32, m03, m13, m23, m33};
        }

        /**
         * @brief Copy constructor
         */
        Matrix4x4(const Matrix4x4 &other) = default;

        /**
         * @brief Move constructor
         */
        Matrix4x4(Matrix4x4 &&other) noexcept = default;

        // ========================================================================
        // Assignment operators
        // ========================================================================

        Matrix4x4 &operator=(const Matrix4x4 &other)     = default;
        Matrix4x4 &operator=(Matrix4x4 &&other) noexcept = default;

        // ========================================================================
        // Accessors
        // ========================================================================

        /**
         * @brief Get the underlying data array (column-major)
         * @return Reference to the internal data array
         */
        const std::array<double, 16> &getData() const { return data_; }

        /**
         * @brief Access element by column-major index
         */
        double &operator[](size_t index) { return data_[index]; }

        const double &operator[](size_t index) const { return data_[index]; }

        /**
         * @brief Access element by row and column
         * @param row Row index (0-3)
         * @param col Column index (0-3)
         */
        double &at(size_t row, size_t col) { return data_[col * 4 + row]; }

        const double &at(size_t row, size_t col) const { return data_[col * 4 + row]; }

        // ========================================================================
        // Matrix operations
        // ========================================================================

        /**
         * @brief Matrix multiplication
         * @param other Matrix to multiply with
         * @return Result of this * other
         */
        Matrix4x4 multiply(const Matrix4x4 &other) const {
            Matrix4x4 result;

            for (size_t row = 0; row < 4; ++row) {
                for (size_t col = 0; col < 4; ++col) {
                    double sum = 0.0;
                    for (size_t k = 0; k < 4; ++k) {
                        sum += at(row, k) * other.at(k, col);
                    }
                    result.at(row, col) = sum;
                }
            }

            return result;
        }

        /**
         * @brief Transform a 3D point (treats as homogeneous with w=1)
         * @param point Point to transform
         * @return Transformed point
         */
        Vector3D transform(const Vector3D &point) const {
            double x = at(0, 0) * point.x + at(0, 1) * point.y + at(0, 2) * point.z + at(0, 3);
            double y = at(1, 0) * point.x + at(1, 1) * point.y + at(1, 2) * point.z + at(1, 3);
            double z = at(2, 0) * point.x + at(2, 1) * point.y + at(2, 2) * point.z + at(2, 3);
            double w = at(3, 0) * point.x + at(3, 1) * point.y + at(3, 2) * point.z + at(3, 3);

            if (std::abs(w - 1.0) > 1e-9) {
                return Vector3D(x / w, y / w, z / w);
            }
            return Vector3D(x, y, z);
        }

        /**
         * @brief Transform a direction vector (treats as homogeneous with w=0)
         * @param direction Direction to transform
         * @return Transformed direction (not translated)
         */
        Vector3D transformDirection(const Vector3D &direction) const {
            double x = at(0, 0) * direction.x + at(0, 1) * direction.y + at(0, 2) * direction.z;
            double y = at(1, 0) * direction.x + at(1, 1) * direction.y + at(1, 2) * direction.z;
            double z = at(2, 0) * direction.x + at(2, 1) * direction.y + at(2, 2) * direction.z;

            return Vector3D(x, y, z);
        }

        /**
         * @brief Return the transpose of this matrix
         * @return Transposed matrix
         */
        Matrix4x4 transpose() const {
            Matrix4x4 result;
            for (size_t row = 0; row < 4; ++row) {
                for (size_t col = 0; col < 4; ++col) {
                    result.at(col, row) = at(row, col);
                }
            }
            return result;
        }

        /**
         * @brief Calculate the determinant of the matrix
         * @return Determinant value
         */
        double determinant() const {
            // Using Laplace expansion along first row
            double det = 0.0;

            for (size_t col = 0; col < 4; ++col) {
                det += (col % 2 == 0 ? 1 : -1) * at(0, col) * minor(0, col);
            }

            return det;
        }

        /**
         * @brief Calculate the inverse of the matrix
         * @return Inverse matrix, or empty optional if not invertible
         */
        std::optional<Matrix4x4> inverse() const {
            double det = determinant();

            if (std::abs(det) < 1e-10) {
                return std::nullopt; // Matrix is singular
            }

            Matrix4x4 result;

            // Calculate cofactor matrix
            for (size_t row = 0; row < 4; ++row) {
                for (size_t col = 0; col < 4; ++col) {
                    double cofactor     = ((row + col) % 2 == 0 ? 1 : -1) * minor(row, col);
                    result.at(col, row) = cofactor / det; // Transpose while building
                }
            }

            return result;
        }

        /**
         * @brief Extract the translation component
         * @return Translation vector
         */
        Vector3D getTranslation() const { return Vector3D(at(0, 3), at(1, 3), at(2, 3)); }

        /**
         * @brief Extract the scale components (assuming no rotation)
         * @return Scale vector
         */
        Vector3D getScale() const {
            double sx = Vector3D(at(0, 0), at(1, 0), at(2, 0)).length();
            double sy = Vector3D(at(0, 1), at(1, 1), at(2, 1)).length();
            double sz = Vector3D(at(0, 2), at(1, 2), at(2, 2)).length();
            return Vector3D(sx, sy, sz);
        }

        /**
         * @brief Convert to string representation
         */
        std::string toString() const {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3);

            for (size_t row = 0; row < 4; ++row) {
                oss << "[ ";
                for (size_t col = 0; col < 4; ++col) {
                    oss << std::setw(8) << at(row, col);
                    if (col < 3)
                        oss << ", ";
                }
                oss << " ]";
                if (row < 3)
                    oss << "\n";
            }

            return oss.str();
        }

        // ========================================================================
        // Operators
        // ========================================================================

        Matrix4x4 operator*(const Matrix4x4 &other) const { return multiply(other); }

        Vector3D operator*(const Vector3D &point) const { return transform(point); }

        Matrix4x4 &operator*=(const Matrix4x4 &other) {
            *this = multiply(other);
            return *this;
        }

        bool operator==(const Matrix4x4 &other) const {
            const double epsilon = 1e-9;
            for (size_t i = 0; i < 16; ++i) {
                if (std::abs(data_[i] - other.data_[i]) >= epsilon) {
                    return false;
                }
            }
            return true;
        }

        bool operator!=(const Matrix4x4 &other) const { return !(*this == other); }

        // ========================================================================
        // Static factory methods
        // ========================================================================

        /**
         * @brief Create identity matrix
         */
        Matrix4x4 identity() { return Matrix4x4(); }

        /**
         * @brief Create translation matrix
         * @param translation Translation vector
         */
        static Matrix4x4 translation(const Vector3D &translation) {
            return Matrix4x4(1.0, 0.0, 0.0, translation.x, 0.0, 1.0, 0.0, translation.y, 0.0, 0.0,
                             1.0, translation.z, 0.0, 0.0, 0.0, 1.0);
        }

        /**
         * @brief Create scale matrix
         * @param scale Scale vector
         */
        static Matrix4x4 scale(const Vector3D &scale) {
            return Matrix4x4(scale.x, 0.0, 0.0, 0.0, 0.0, scale.y, 0.0, 0.0, 0.0, 0.0, scale.z, 0.0,
                             0.0, 0.0, 0.0, 1.0);
        }

        /**
         * @brief Create uniform scale matrix
         * @param scale Uniform scale factor
         */
        static Matrix4x4 scale(double scale) { return Matrix4x4::scale(Vector3D::uniform(scale)); }

        /**
         * @brief Create rotation matrix around X axis
         * @param angle Angle in radians
         */
        static Matrix4x4 rotationX(double angle) {
            double c = std::cos(angle);
            double s = std::sin(angle);

            return Matrix4x4(1.0, 0.0, 0.0, 0.0, 0.0, c, -s, 0.0, 0.0, s, c, 0.0, 0.0, 0.0, 0.0,
                             1.0);
        }

        /**
         * @brief Create rotation matrix around Y axis
         * @param angle Angle in radians
         */
        static Matrix4x4 rotationY(double angle) {
            double c = std::cos(angle);
            double s = std::sin(angle);

            return Matrix4x4(c, 0.0, s, 0.0, 0.0, 1.0, 0.0, 0.0, -s, 0.0, c, 0.0, 0.0, 0.0, 0.0,
                             1.0);
        }

        /**
         * @brief Create rotation matrix around Z axis
         * @param angle Angle in radians
         */
        static Matrix4x4 rotationZ(double angle) {
            double c = std::cos(angle);
            double s = std::sin(angle);

            return Matrix4x4(c, -s, 0.0, 0.0, s, c, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
                             1.0);
        }

        /**
         * @brief Create perspective projection matrix
         * @param fovY Vertical field of view in radians
         * @param aspect Aspect ratio (width/height)
         * @param near Near clipping plane
         * @param far Far clipping plane
         */
        static Matrix4x4 perspective(double fovY, double aspect, double near, double far) {
            double f = 1.0 / std::tan(fovY / 2.0);

            return Matrix4x4(f / aspect, 0.0, 0.0, 0.0, 0.0, f, 0.0, 0.0, 0.0, 0.0,
                             (far + near) / (near - far), (2.0 * far * near) / (near - far), 0.0,
                             0.0, -1.0, 0.0);
        }

        /**
         * @brief Create orthographic projection matrix
         * @param left Left plane
         * @param right Right plane
         * @param bottom Bottom plane
         * @param top Top plane
         * @param near Near plane
         * @param far Far plane
         */
        static Matrix4x4 orthographic(double left, double right, double bottom, double top,
                                      double near, double far) {
            return Matrix4x4(2.0 / (right - left), 0.0, 0.0, -(right + left) / (right - left), 0.0,
                             2.0 / (top - bottom), 0.0, -(top + bottom) / (top - bottom), 0.0, 0.0,
                             -2.0 / (far - near), -(far + near) / (far - near), 0.0, 0.0, 0.0, 1.0);
        }

        /**
         * @brief Create look-at view matrix
         * @param eye Camera position
         * @param center Target position
         * @param up Up vector
         */
        static Matrix4x4 lookAt(const Vector3D &eye, const Vector3D &center, const Vector3D &up) {
            Vector3D f = (center - eye).normalize();
            Vector3D s = f.cross(up).normalize();
            Vector3D u = s.cross(f);

            Matrix4x4 result;
            result.at(0, 0) = s.x;
            result.at(0, 1) = s.y;
            result.at(0, 2) = s.z;
            result.at(1, 0) = u.x;
            result.at(1, 1) = u.y;
            result.at(1, 2) = u.z;
            result.at(2, 0) = -f.x;
            result.at(2, 1) = -f.y;
            result.at(2, 2) = -f.z;
            result.at(0, 3) = -s.dot(eye);
            result.at(1, 3) = -u.dot(eye);
            result.at(2, 3) = f.dot(eye);

            return result;
        }

    private:
        /**
         * @brief Calculate minor (determinant of 3x3 submatrix)
         */
        double minor(size_t row, size_t col) const {
            double sub[9];
            size_t idx = 0;

            for (size_t r = 0; r < 4; ++r) {
                if (r == row)
                    continue;
                for (size_t c = 0; c < 4; ++c) {
                    if (c == col)
                        continue;
                    sub[idx++] = at(r, c);
                }
            }

            // Calculate 3x3 determinant
            return sub[0] * (sub[4] * sub[8] - sub[5] * sub[7]) -
                   sub[1] * (sub[3] * sub[8] - sub[5] * sub[6]) +
                   sub[2] * (sub[3] * sub[7] - sub[4] * sub[6]);
        }
    };

    // ============================================================================
    // Stream output operator
    // ============================================================================

    inline std::ostream &operator<<(std::ostream &os, const Matrix4x4 &mat) {
        os << mat.toString();
        return os;
    }

} // namespace mylib
