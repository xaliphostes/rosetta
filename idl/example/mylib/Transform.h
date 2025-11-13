// ============================================================================
// mylib/Transform.h
// 
// 3D transformation class combining position, rotation, and scale
// ============================================================================
#pragma once

#include "Vector3D.h"
#include "Matrix4x4.h"
#include <cmath>

namespace mylib {

/**
 * @brief 3D transformation with position, rotation, and scale
 * 
 * Represents a complete 3D transformation as separate components:
 * - Position (translation)
 * - Rotation (as Euler angles or matrix)
 * - Scale
 * 
 * Can be converted to a transformation matrix for actual transformations.
 */
class Transform {
private:
    Vector3D position_;     ///< Position in 3D space
    Vector3D eulerAngles_;  ///< Rotation as Euler angles (radians)
    Vector3D scale_;        ///< Scale factors (sx, sy, sz)
    Matrix4x4 rotationMatrix_; ///< Cached rotation matrix
    bool matrixDirty_;      ///< Flag indicating if matrix needs update

public:
    // ========================================================================
    // Constructors
    // ========================================================================
    
    /**
     * @brief Default constructor - identity transform
     */
    Transform()
        : position_(0.0, 0.0, 0.0)
        , eulerAngles_(0.0, 0.0, 0.0)
        , scale_(1.0, 1.0, 1.0)
        , rotationMatrix_(Matrix4x4().identity())
        , matrixDirty_(false)
    {}
    
    /**
     * @brief Constructor with position
     * @param position Initial position
     */
    explicit Transform(const Vector3D& position)
        : position_(position)
        , eulerAngles_(0.0, 0.0, 0.0)
        , scale_(1.0, 1.0, 1.0)
        , rotationMatrix_(Matrix4x4().identity())
        , matrixDirty_(false)
    {}
    
    /**
     * @brief Constructor with position and rotation
     * @param position Initial position
     * @param rotation Initial rotation (Euler angles in radians)
     */
    Transform(const Vector3D& position, const Vector3D& rotation)
        : position_(position)
        , eulerAngles_(rotation)
        , scale_(1.0, 1.0, 1.0)
        , rotationMatrix_(Matrix4x4().identity())
        , matrixDirty_(true)
    {}
    
    /**
     * @brief Constructor with position, rotation, and scale
     * @param position Initial position
     * @param rotation Initial rotation (Euler angles in radians)
     * @param scale Initial scale
     */
    Transform(const Vector3D& position, const Vector3D& rotation, const Vector3D& scale)
        : position_(position)
        , eulerAngles_(rotation)
        , scale_(scale)
        , rotationMatrix_(Matrix4x4().identity())
        , matrixDirty_(true)
    {}
    
    /**
     * @brief Copy constructor
     */
    Transform(const Transform& other) = default;
    
    /**
     * @brief Move constructor
     */
    Transform(Transform&& other) noexcept = default;

    // ========================================================================
    // Assignment operators
    // ========================================================================
    
    Transform& operator=(const Transform& other) = default;
    Transform& operator=(Transform&& other) noexcept = default;

    // ========================================================================
    // Position accessors
    // ========================================================================
    
    /**
     * @brief Get the position
     */
    Vector3D getPosition() const {
        return position_;
    }
    
    /**
     * @brief Set the position
     */
    void setPosition(const Vector3D& position) {
        position_ = position;
    }
    
    /**
     * @brief Translate by a vector
     */
    void translate(const Vector3D& offset) {
        position_ += offset;
    }

    // ========================================================================
    // Rotation accessors
    // ========================================================================
    
    /**
     * @brief Get rotation as Euler angles (radians)
     */
    Vector3D getEulerAngles() const {
        return eulerAngles_;
    }
    
    /**
     * @brief Set rotation using Euler angles (radians)
     * @param angles Rotation angles (x, y, z) in radians
     */
    void setEulerAngles(const Vector3D& angles) {
        eulerAngles_ = angles;
        matrixDirty_ = true;
    }
    
    /**
     * @brief Get rotation as a matrix
     */
    Matrix4x4 getRotation() const {
        if (matrixDirty_) {
            const_cast<Transform*>(this)->updateRotationMatrix();
        }
        return rotationMatrix_;
    }
    
    /**
     * @brief Set rotation using a matrix
     */
    void setRotation(const Matrix4x4& rotation) {
        rotationMatrix_ = rotation;
        matrixDirty_ = false;
        // Note: eulerAngles_ are now out of sync
        // Would need to extract Euler angles from matrix if needed
    }
    
    /**
     * @brief Rotate around X axis
     * @param angle Angle in radians
     */
    void rotateX(double angle) {
        eulerAngles_.x += angle;
        matrixDirty_ = true;
    }
    
    /**
     * @brief Rotate around Y axis
     * @param angle Angle in radians
     */
    void rotateY(double angle) {
        eulerAngles_.y += angle;
        matrixDirty_ = true;
    }
    
    /**
     * @brief Rotate around Z axis
     * @param angle Angle in radians
     */
    void rotateZ(double angle) {
        eulerAngles_.z += angle;
        matrixDirty_ = true;
    }

    // ========================================================================
    // Scale accessors
    // ========================================================================
    
    /**
     * @brief Get the scale
     */
    Vector3D getScale() const {
        return scale_;
    }
    
    /**
     * @brief Set the scale
     */
    void setScale(const Vector3D& scale) {
        scale_ = scale;
    }
    
    /**
     * @brief Set uniform scale
     */
    void setScale(double uniformScale) {
        scale_ = Vector3D::uniform(uniformScale);
    }

    // ========================================================================
    // Forward/Right/Up vectors
    // ========================================================================
    
    /**
     * @brief Get the forward vector (local -Z axis)
     */
    Vector3D getForward() const {
        Matrix4x4 rot = getRotation();
        return Vector3D(rot.at(0, 2), rot.at(1, 2), rot.at(2, 2)).normalize();
    }
    
    /**
     * @brief Get the right vector (local +X axis)
     */
    Vector3D getRight() const {
        Matrix4x4 rot = getRotation();
        return Vector3D(rot.at(0, 0), rot.at(1, 0), rot.at(2, 0)).normalize();
    }
    
    /**
     * @brief Get the up vector (local +Y axis)
     */
    Vector3D getUp() const {
        Matrix4x4 rot = getRotation();
        return Vector3D(rot.at(0, 1), rot.at(1, 1), rot.at(2, 1)).normalize();
    }

    // ========================================================================
    // Matrix conversion
    // ========================================================================
    
    /**
     * @brief Convert to transformation matrix
     * @return 4x4 transformation matrix (TRS order: Translation * Rotation * Scale)
     */
    Matrix4x4 toMatrix() const {
        if (matrixDirty_) {
            const_cast<Transform*>(this)->updateRotationMatrix();
        }
        
        // Combine: Translation * Rotation * Scale
        Matrix4x4 translationMatrix = Matrix4x4::translation(position_);
        Matrix4x4 scaleMatrix = Matrix4x4::scale(scale_);
        
        return translationMatrix * rotationMatrix_ * scaleMatrix;
    }
    
    /**
     * @brief Get the inverse transformation matrix
     */
    Matrix4x4 toInverseMatrix() const {
        auto inv = toMatrix().inverse();
        return inv.value_or(Matrix4x4().identity());
    }

    // ========================================================================
    // Transform operations
    // ========================================================================
    
    /**
     * @brief Transform a point (applies full transformation)
     * @param point Point to transform
     * @return Transformed point
     */
    Vector3D transformPoint(const Vector3D& point) const {
        return toMatrix().transform(point);
    }
    
    /**
     * @brief Transform a direction vector (ignores translation)
     * @param direction Direction to transform
     * @return Transformed direction
     */
    Vector3D transformDirection(const Vector3D& direction) const {
        if (matrixDirty_) {
            const_cast<Transform*>(this)->updateRotationMatrix();
        }
        
        // Apply rotation and scale
        Vector3D scaled(
            direction.x * scale_.x,
            direction.y * scale_.y,
            direction.z * scale_.z
        );
        
        return rotationMatrix_.transformDirection(scaled);
    }
    
    /**
     * @brief Inverse transform a point
     */
    Vector3D inverseTransformPoint(const Vector3D& point) const {
        return toInverseMatrix().transform(point);
    }
    
    /**
     * @brief Inverse transform a direction
     */
    Vector3D inverseTransformDirection(const Vector3D& direction) const {
        if (matrixDirty_) {
            const_cast<Transform*>(this)->updateRotationMatrix();
        }
        
        // Inverse rotation
        Matrix4x4 invRot = rotationMatrix_.transpose();
        Vector3D rotated = invRot.transformDirection(direction);
        
        // Inverse scale
        return Vector3D(
            rotated.x / scale_.x,
            rotated.y / scale_.y,
            rotated.z / scale_.z
        );
    }

    // ========================================================================
    // Combination
    // ========================================================================
    
    /**
     * @brief Combine this transform with another (this * other)
     */
    Transform combine(const Transform& other) const {
        Matrix4x4 combined = toMatrix() * other.toMatrix();
        
        // Extract components from combined matrix
        Transform result;
        result.position_ = combined.getTranslation();
        result.scale_ = combined.getScale();
        // Note: Rotation extraction from matrix is complex
        // For simplicity, store the combined rotation matrix
        result.rotationMatrix_ = combined;
        result.matrixDirty_ = false;
        
        return result;
    }
    
    /**
     * @brief Get the inverse transform
     */
    Transform inverse() const {
        Matrix4x4 invMatrix = toInverseMatrix();
        
        Transform result;
        result.position_ = invMatrix.getTranslation();
        result.scale_ = invMatrix.getScale();
        result.rotationMatrix_ = invMatrix;
        result.matrixDirty_ = false;
        
        return result;
    }

    // ========================================================================
    // Interpolation
    // ========================================================================
    
    /**
     * @brief Linear interpolation between transforms
     * @param other Target transform
     * @param t Interpolation factor (0.0 to 1.0)
     */
    Transform lerp(const Transform& other, double t) const {
        Transform result;
        result.position_ = position_.lerp(other.position_, t);
        result.eulerAngles_ = eulerAngles_.lerp(other.eulerAngles_, t);
        result.scale_ = scale_.lerp(other.scale_, t);
        result.matrixDirty_ = true;
        return result;
    }

    // ========================================================================
    // Utility
    // ========================================================================
    
    /**
     * @brief Reset to identity transform
     */
    void reset() {
        position_ = Vector3D::zero();
        eulerAngles_ = Vector3D::zero();
        scale_ = Vector3D::uniform(1.0);
        rotationMatrix_ = Matrix4x4().identity();
        matrixDirty_ = false;
    }
    
    /**
     * @brief Check if this is an identity transform
     */
    bool isIdentity() const {
        const double epsilon = 1e-9;
        return position_.lengthSquared() < epsilon &&
               eulerAngles_.lengthSquared() < epsilon &&
               (scale_ - Vector3D::uniform(1.0)).lengthSquared() < epsilon;
    }
    
    /**
     * @brief Convert to string representation
     */
    std::string toString() const {
        std::ostringstream oss;
        oss << "Transform:\n";
        oss << "  Position: " << position_.toString() << "\n";
        oss << "  Rotation: " << eulerAngles_.toString() << " rad\n";
        oss << "  Scale: " << scale_.toString();
        return oss.str();
    }

    // ========================================================================
    // Operators
    // ========================================================================
    
    /**
     * @brief Combine transforms (multiplication)
     */
    Transform operator*(const Transform& other) const {
        return combine(other);
    }
    
    /**
     * @brief Transform a point
     */
    Vector3D operator*(const Vector3D& point) const {
        return transformPoint(point);
    }
    
    bool operator==(const Transform& other) const {
        return position_ == other.position_ &&
               eulerAngles_ == other.eulerAngles_ &&
               scale_ == other.scale_;
    }
    
    bool operator!=(const Transform& other) const {
        return !(*this == other);
    }

    // ========================================================================
    // Static factory methods
    // ========================================================================
    
    /**
     * @brief Create identity transform
     */
    static Transform identity() {
        return Transform();
    }
    
    /**
     * @brief Create transform with only translation
     */
    static Transform fromPosition(const Vector3D& position) {
        return Transform(position);
    }
    
    /**
     * @brief Create transform with translation and rotation
     */
    static Transform fromPositionRotation(const Vector3D& position, const Vector3D& rotation) {
        return Transform(position, rotation);
    }
    
    /**
     * @brief Create transform from matrix
     */
    static Transform fromMatrix(const Matrix4x4& matrix) {
        Transform result;
        result.position_ = matrix.getTranslation();
        result.scale_ = matrix.getScale();
        result.rotationMatrix_ = matrix;
        result.matrixDirty_ = false;
        return result;
    }
    
    /**
     * @brief Create look-at transform
     * @param position Camera position
     * @param target Target to look at
     * @param up Up vector (default: Y-up)
     */
    static Transform lookAt(const Vector3D& position, 
                           const Vector3D& target,
                           const Vector3D& up = Vector3D::unitY()) {
        Matrix4x4 lookAtMatrix = Matrix4x4::lookAt(position, target, up);
        return fromMatrix(lookAtMatrix);
    }

private:
    /**
     * @brief Update the rotation matrix from Euler angles
     */
    void updateRotationMatrix() {
        // Apply rotations in ZYX order (yaw, pitch, roll)
        Matrix4x4 rotX = Matrix4x4::rotationX(eulerAngles_.x);
        Matrix4x4 rotY = Matrix4x4::rotationY(eulerAngles_.y);
        Matrix4x4 rotZ = Matrix4x4::rotationZ(eulerAngles_.z);
        
        rotationMatrix_ = rotZ * rotY * rotX;
        matrixDirty_ = false;
    }
};

// ============================================================================
// Stream output operator
// ============================================================================

inline std::ostream& operator<<(std::ostream& os, const Transform& transform) {
    os << transform.toString();
    return os;
}

} // namespace mylib
