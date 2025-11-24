// ============================================================================
// Rosetta Registration for 3D Scene Objects
// ============================================================================
#pragma once

#include "scene_objects.h"
#include <rosetta/rosetta.h>

namespace rosetta::qt3d {

    /**
     * @brief Register all 3D scene classes with Rosetta
     *
     * Call this at application startup before using property editors
     */
    inline void registerRosettaClasses() {
        using namespace rosetta::core;

        // ====================================================================
        // Transform3D
        // ====================================================================
        Registry::instance()
            .register_class<Transform3D>("Transform3D")
            .constructor<>()
            // Position
            .property("positionX", &Transform3D::positionX, &Transform3D::setPositionX)
            .property("positionY", &Transform3D::positionY, &Transform3D::setPositionY)
            .property("positionZ", &Transform3D::positionZ, &Transform3D::setPositionZ)
            // Rotation
            .property("rotationX", &Transform3D::rotationX, &Transform3D::setRotationX)
            .property("rotationY", &Transform3D::rotationY, &Transform3D::setRotationY)
            .property("rotationZ", &Transform3D::rotationZ, &Transform3D::setRotationZ)
            // Scale
            .property("scaleX", &Transform3D::scaleX, &Transform3D::setScaleX)
            .property("scaleY", &Transform3D::scaleY, &Transform3D::setScaleY)
            .property("scaleZ", &Transform3D::scaleZ, &Transform3D::setScaleZ)
            // Methods
            .method("reset", &Transform3D::reset)
            .method("translate", &Transform3D::translate)
            .method("rotate", &Transform3D::rotate)
            .method("setUniformScale", &Transform3D::setUniformScale);

        // ====================================================================
        // Material3D
        // ====================================================================
        Registry::instance()
            .register_class<Material3D>("Material3D")
            .constructor<>()
            // Color
            .property("red", &Material3D::red, &Material3D::setRed)
            .property("green", &Material3D::green, &Material3D::setGreen)
            .property("blue", &Material3D::blue, &Material3D::setBlue)
            .property("alpha", &Material3D::alpha, &Material3D::setAlpha)
            // PBR
            .property("metalness", &Material3D::metalness, &Material3D::setMetalness)
            .property("roughness", &Material3D::roughness, &Material3D::setRoughness)
            .property("ambientOcclusion", &Material3D::ambientOcclusion,
                      &Material3D::setAmbientOcclusion)
            // Presets
            .method("setMetal", &Material3D::setMetal)
            .method("setPlastic", &Material3D::setPlastic)
            .method("setRubber", &Material3D::setRubber)
            .method("setGlass", &Material3D::setGlass);

        // ====================================================================
        // SceneObject3D (base class)
        // ====================================================================
        // Note: SceneObject3D is a QObject with Q_PROPERTY declarations
        // We register the public getter/setter pairs for Rosetta introspection
        Registry::instance()
            .register_class<SceneObject3D>("SceneObject3D")
            // Visibility
            .property("visible", &SceneObject3D::visible, &SceneObject3D::setVisible)
            // Transform properties (delegated to internal Transform3D)
            .property("positionX", &SceneObject3D::positionX, &SceneObject3D::setPositionX)
            .property("positionY", &SceneObject3D::positionY, &SceneObject3D::setPositionY)
            .property("positionZ", &SceneObject3D::positionZ, &SceneObject3D::setPositionZ)
            .property("rotationX", &SceneObject3D::rotationX, &SceneObject3D::setRotationX)
            .property("rotationY", &SceneObject3D::rotationY, &SceneObject3D::setRotationY)
            .property("rotationZ", &SceneObject3D::rotationZ, &SceneObject3D::setRotationZ)
            .property("scaleX", &SceneObject3D::scaleX, &SceneObject3D::setScaleX)
            .property("scaleY", &SceneObject3D::scaleY, &SceneObject3D::setScaleY)
            .property("scaleZ", &SceneObject3D::scaleZ, &SceneObject3D::setScaleZ)
            // Material properties (delegated to internal Material3D)
            .property("colorR", &SceneObject3D::colorR, &SceneObject3D::setColorR)
            .property("colorG", &SceneObject3D::colorG, &SceneObject3D::setColorG)
            .property("colorB", &SceneObject3D::colorB, &SceneObject3D::setColorB)
            .property("metalness", &SceneObject3D::metalness, &SceneObject3D::setMetalness)
            .property("roughness", &SceneObject3D::roughness, &SceneObject3D::setRoughness)
            // Methods
            .method("resetTransform", &SceneObject3D::resetTransform)
            .method("setUniformScale", &SceneObject3D::setUniformScale);

        // ====================================================================
        // Primitive3D
        // ====================================================================
        // Primitive3D inherits from SceneObject3D - inherit_from copies all
        // base class properties and methods automatically
        Registry::instance()
            .register_class<Primitive3D>("Primitive3D")
            .inherits_from<SceneObject3D>("SceneObject3D")
            // Primitive-specific properties only
            .property("segments", &Primitive3D::segments, &Primitive3D::setSegments)
            .property("rings", &Primitive3D::rings, &Primitive3D::setRings);

        // ====================================================================
        // Light3D
        // ====================================================================
        Registry::instance()
            .register_class<Light3D>("Light3D")
            .inherits_from<SceneObject3D>("SceneObject3D")
            // Light-specific properties
            .property("intensity", &Light3D::intensity, &Light3D::setIntensity)
            .property("range", &Light3D::range, &Light3D::setRange)
            .property("spotAngle", &Light3D::spotAngle, &Light3D::setSpotAngle)
            .property("castShadows", &Light3D::castShadows, &Light3D::setCastShadows);

        // ====================================================================
        // Camera3D
        // ====================================================================
        Registry::instance()
            .register_class<Camera3D>("Camera3D")
            .inherits_from<SceneObject3D>("SceneObject3D")
            // Camera-specific properties
            .property("fieldOfView", &Camera3D::fieldOfView, &Camera3D::setFieldOfView)
            .property("nearPlane", &Camera3D::nearPlane, &Camera3D::setNearPlane)
            .property("farPlane", &Camera3D::farPlane, &Camera3D::setFarPlane)
            .property("aspectRatio", &Camera3D::aspectRatio, &Camera3D::setAspectRatio)
            .property("orthographic", &Camera3D::orthographic, &Camera3D::setOrthographic)
            .property("orthoSize", &Camera3D::orthoSize, &Camera3D::setOrthoSize)
            // Camera methods
            .method("lookAt", &Camera3D::lookAt);

        // ====================================================================
        // Scene3D
        // ====================================================================
        Registry::instance()
            .register_class<Scene3D>("Scene3D")
            // Scene properties
            .property("ambientIntensity", &Scene3D::ambientIntensity, &Scene3D::setAmbientIntensity)
            .property("showGrid", &Scene3D::showGrid, &Scene3D::setShowGrid)
            .property("showAxes", &Scene3D::showAxes, &Scene3D::setShowAxes)
            .readonly_property("objectCount", &Scene3D::objectCount)
            // Scene methods
            .method("clearObjects", &Scene3D::clearObjects);
    }

    // ============================================================================
    // Property Editor Bridge - Connect Rosetta to QML
    // ============================================================================

    /**
     * @brief Bridge class to expose Rosetta property editing to QML
     */
    class PropertyEditorBridge : public QObject {
        Q_OBJECT
        Q_PROPERTY(QStringList fieldNames READ fieldNames NOTIFY objectChanged)
        Q_PROPERTY(QStringList methodNames READ methodNames NOTIFY objectChanged)

    public:
        explicit PropertyEditorBridge(QObject *parent = nullptr) : QObject(parent) {}

        /**
         * @brief Set the object to edit (by class name and pointer)
         */
        Q_INVOKABLE void setObject(const QString &className, QObject *obj) {
            class_name_     = className.toStdString();
            current_object_ = obj;
            emit objectChanged();
        }

        QStringList fieldNames() const {
            QStringList names;
            if (class_name_.empty())
                return names;

            auto *holder = core::Registry::instance().get_by_name(class_name_);
            if (holder) {
                // Note: Would need to extend Registry to get field names from holder
                // For now, return empty - actual implementation would iterate fields
            }
            return names;
        }

        QStringList methodNames() const {
            QStringList names;
            if (class_name_.empty())
                return names;

            auto *holder = core::Registry::instance().get_by_name(class_name_);
            if (holder) {
                for (const auto &m : holder->get_methods()) {
                    names.append(QString::fromStdString(m));
                }
            }
            return names;
        }

        /**
         * @brief Get a field value as QVariant (for QML binding)
         */
        Q_INVOKABLE QVariant getField(const QString &fieldName) const {
            // Implementation would use Rosetta to get field
            // Convert core::Any to QVariant
            return QVariant();
        }

        /**
         * @brief Set a field value from QVariant (from QML)
         */
        Q_INVOKABLE void setField(const QString &fieldName, const QVariant &value) {
            // Implementation would convert QVariant to core::Any
            // Then use Rosetta to set field
            emit fieldChanged(fieldName);
        }

        /**
         * @brief Invoke a method with arguments
         */
        Q_INVOKABLE QVariant invokeMethod(const QString &methodName, const QVariantList &args) {
            // Implementation would convert args and invoke via Rosetta
            return QVariant();
        }

    signals:
        void objectChanged();
        void fieldChanged(const QString &fieldName);

    private:
        std::string class_name_;
        QObject    *current_object_ = nullptr;
    };

} // namespace rosetta::qt3d