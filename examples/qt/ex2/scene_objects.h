// ============================================================================
// Qt6 QML 3D Example with Rosetta Property Editing
// ============================================================================
#pragma once

#include <QColor>
#include <QMatrix4x4>
#include <QObject>
#include <QQmlEngine>
#include <QQuaternion>
#include <QVector3D>

#include <cmath>
#include <string>
#include <vector>

namespace rosetta::qt3d {

    // ============================================================================
    // Transform3D - Position, Rotation, Scale for 3D objects
    // ============================================================================

    class Transform3D {
        Q_GADGET
        Q_PROPERTY(float positionX READ positionX WRITE setPositionX)
        Q_PROPERTY(float positionY READ positionY WRITE setPositionY)
        Q_PROPERTY(float positionZ READ positionZ WRITE setPositionZ)
        Q_PROPERTY(float rotationX READ rotationX WRITE setRotationX)
        Q_PROPERTY(float rotationY READ rotationY WRITE setRotationY)
        Q_PROPERTY(float rotationZ READ rotationZ WRITE setRotationZ)
        Q_PROPERTY(float scaleX READ scaleX WRITE setScaleX)
        Q_PROPERTY(float scaleY READ scaleY WRITE setScaleY)
        Q_PROPERTY(float scaleZ READ scaleZ WRITE setScaleZ)

    public:
        Transform3D() = default;

        // Position
        float positionX() const { return position_x_; }
        float positionY() const { return position_y_; }
        float positionZ() const { return position_z_; }

        void setPositionX(float x) { position_x_ = x; }
        void setPositionY(float y) { position_y_ = y; }
        void setPositionZ(float z) { position_z_ = z; }

        QVector3D position() const { return QVector3D(position_x_, position_y_, position_z_); }
        void      setPosition(const QVector3D &pos) {
            position_x_ = pos.x();
            position_y_ = pos.y();
            position_z_ = pos.z();
        }

        // Rotation (Euler angles in degrees)
        float rotationX() const { return rotation_x_; }
        float rotationY() const { return rotation_y_; }
        float rotationZ() const { return rotation_z_; }

        void setRotationX(float x) { rotation_x_ = x; }
        void setRotationY(float y) { rotation_y_ = y; }
        void setRotationZ(float z) { rotation_z_ = z; }

        QVector3D rotation() const { return QVector3D(rotation_x_, rotation_y_, rotation_z_); }
        void      setRotation(const QVector3D &rot) {
            rotation_x_ = rot.x();
            rotation_y_ = rot.y();
            rotation_z_ = rot.z();
        }

        QQuaternion quaternion() const {
            return QQuaternion::fromEulerAngles(rotation_x_, rotation_y_, rotation_z_);
        }

        // Scale
        float scaleX() const { return scale_x_; }
        float scaleY() const { return scale_y_; }
        float scaleZ() const { return scale_z_; }

        void setScaleX(float x) { scale_x_ = x; }
        void setScaleY(float y) { scale_y_ = y; }
        void setScaleZ(float z) { scale_z_ = z; }

        QVector3D scale() const { return QVector3D(scale_x_, scale_y_, scale_z_); }
        void      setScale(const QVector3D &s) {
            scale_x_ = s.x();
            scale_y_ = s.y();
            scale_z_ = s.z();
        }
        void setUniformScale(float s) { scale_x_ = scale_y_ = scale_z_ = s; }

        // Matrix
        QMatrix4x4 matrix() const {
            QMatrix4x4 m;
            m.translate(position());
            m.rotate(quaternion());
            m.scale(scale());
            return m;
        }

        // Utility
        void reset() {
            position_x_ = position_y_ = position_z_ = 0.0f;
            rotation_x_ = rotation_y_ = rotation_z_ = 0.0f;
            scale_x_ = scale_y_ = scale_z_ = 1.0f;
        }

        void translate(float dx, float dy, float dz) {
            position_x_ += dx;
            position_y_ += dy;
            position_z_ += dz;
        }

        void rotate(float dx, float dy, float dz) {
            rotation_x_ += dx;
            rotation_y_ += dy;
            rotation_z_ += dz;
        }

    private:
        float position_x_ = 0.0f;
        float position_y_ = 0.0f;
        float position_z_ = 0.0f;
        float rotation_x_ = 0.0f;
        float rotation_y_ = 0.0f;
        float rotation_z_ = 0.0f;
        float scale_x_    = 1.0f;
        float scale_y_    = 1.0f;
        float scale_z_    = 1.0f;
    };

    // ============================================================================
    // Material3D - Material properties for 3D objects
    // ============================================================================

    class Material3D {
        Q_GADGET
        Q_PROPERTY(float red READ red WRITE setRed)
        Q_PROPERTY(float green READ green WRITE setGreen)
        Q_PROPERTY(float blue READ blue WRITE setBlue)
        Q_PROPERTY(float alpha READ alpha WRITE setAlpha)
        Q_PROPERTY(float metalness READ metalness WRITE setMetalness)
        Q_PROPERTY(float roughness READ roughness WRITE setRoughness)
        Q_PROPERTY(float ambientOcclusion READ ambientOcclusion WRITE setAmbientOcclusion)

    public:
        Material3D() = default;

        // Color components (0-1 range)
        float red() const { return red_; }
        float green() const { return green_; }
        float blue() const { return blue_; }
        float alpha() const { return alpha_; }

        void setRed(float r) { red_ = std::clamp(r, 0.0f, 1.0f); }
        void setGreen(float g) { green_ = std::clamp(g, 0.0f, 1.0f); }
        void setBlue(float b) { blue_ = std::clamp(b, 0.0f, 1.0f); }
        void setAlpha(float a) { alpha_ = std::clamp(a, 0.0f, 1.0f); }

        QColor color() const { return QColor::fromRgbF(red_, green_, blue_, alpha_); }
        void   setColor(const QColor &c) {
            red_   = c.redF();
            green_ = c.greenF();
            blue_  = c.blueF();
            alpha_ = c.alphaF();
        }

        // PBR properties
        float metalness() const { return metalness_; }
        float roughness() const { return roughness_; }
        float ambientOcclusion() const { return ambient_occlusion_; }

        void setMetalness(float m) { metalness_ = std::clamp(m, 0.0f, 1.0f); }
        void setRoughness(float r) { roughness_ = std::clamp(r, 0.0f, 1.0f); }
        void setAmbientOcclusion(float ao) { ambient_occlusion_ = std::clamp(ao, 0.0f, 1.0f); }

        // Presets
        void setMetal() {
            metalness_ = 1.0f;
            roughness_ = 0.3f;
        }

        void setPlastic() {
            metalness_ = 0.0f;
            roughness_ = 0.5f;
        }

        void setRubber() {
            metalness_ = 0.0f;
            roughness_ = 0.9f;
        }

        void setGlass() {
            metalness_ = 0.0f;
            roughness_ = 0.1f;
            alpha_     = 0.3f;
        }

    private:
        float red_               = 0.8f;
        float green_             = 0.8f;
        float blue_              = 0.8f;
        float alpha_             = 1.0f;
        float metalness_         = 0.0f;
        float roughness_         = 0.5f;
        float ambient_occlusion_ = 1.0f;
    };

    // ============================================================================
    // SceneObject3D - Base class for all 3D scene objects
    // ============================================================================

    class SceneObject3D : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
        Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
        Q_PROPERTY(float positionX READ positionX WRITE setPositionX NOTIFY transformChanged)
        Q_PROPERTY(float positionY READ positionY WRITE setPositionY NOTIFY transformChanged)
        Q_PROPERTY(float positionZ READ positionZ WRITE setPositionZ NOTIFY transformChanged)
        Q_PROPERTY(float rotationX READ rotationX WRITE setRotationX NOTIFY transformChanged)
        Q_PROPERTY(float rotationY READ rotationY WRITE setRotationY NOTIFY transformChanged)
        Q_PROPERTY(float rotationZ READ rotationZ WRITE setRotationZ NOTIFY transformChanged)
        Q_PROPERTY(float scaleX READ scaleX WRITE setScaleX NOTIFY transformChanged)
        Q_PROPERTY(float scaleY READ scaleY WRITE setScaleY NOTIFY transformChanged)
        Q_PROPERTY(float scaleZ READ scaleZ WRITE setScaleZ NOTIFY transformChanged)
        Q_PROPERTY(float colorR READ colorR WRITE setColorR NOTIFY materialChanged)
        Q_PROPERTY(float colorG READ colorG WRITE setColorG NOTIFY materialChanged)
        Q_PROPERTY(float colorB READ colorB WRITE setColorB NOTIFY materialChanged)
        Q_PROPERTY(float metalness READ metalness WRITE setMetalness NOTIFY materialChanged)
        Q_PROPERTY(float roughness READ roughness WRITE setRoughness NOTIFY materialChanged)

    public:
        explicit SceneObject3D(QObject *parent = nullptr) : QObject(parent) {}

        // Name
        QString name() const { return QString::fromStdString(name_); }
        void    setName(const QString &n) {
            if (name_ != n.toStdString()) {
                name_ = n.toStdString();
                emit nameChanged();
            }
        }

        // Visibility
        bool visible() const { return visible_; }
        void setVisible(bool v) {
            if (visible_ != v) {
                visible_ = v;
                emit visibleChanged();
            }
        }

        // Transform accessors
        float positionX() const { return transform_.positionX(); }
        float positionY() const { return transform_.positionY(); }
        float positionZ() const { return transform_.positionZ(); }
        float rotationX() const { return transform_.rotationX(); }
        float rotationY() const { return transform_.rotationY(); }
        float rotationZ() const { return transform_.rotationZ(); }
        float scaleX() const { return transform_.scaleX(); }
        float scaleY() const { return transform_.scaleY(); }
        float scaleZ() const { return transform_.scaleZ(); }

        void setPositionX(float v) {
            transform_.setPositionX(v);
            emit transformChanged();
        }
        void setPositionY(float v) {
            transform_.setPositionY(v);
            emit transformChanged();
        }
        void setPositionZ(float v) {
            transform_.setPositionZ(v);
            emit transformChanged();
        }
        void setRotationX(float v) {
            transform_.setRotationX(v);
            emit transformChanged();
        }
        void setRotationY(float v) {
            transform_.setRotationY(v);
            emit transformChanged();
        }
        void setRotationZ(float v) {
            transform_.setRotationZ(v);
            emit transformChanged();
        }
        void setScaleX(float v) {
            transform_.setScaleX(v);
            emit transformChanged();
        }
        void setScaleY(float v) {
            transform_.setScaleY(v);
            emit transformChanged();
        }
        void setScaleZ(float v) {
            transform_.setScaleZ(v);
            emit transformChanged();
        }

        // Material accessors
        float colorR() const { return material_.red(); }
        float colorG() const { return material_.green(); }
        float colorB() const { return material_.blue(); }
        float metalness() const { return material_.metalness(); }
        float roughness() const { return material_.roughness(); }

        void setColorR(float v) {
            material_.setRed(v);
            emit materialChanged();
        }
        void setColorG(float v) {
            material_.setGreen(v);
            emit materialChanged();
        }
        void setColorB(float v) {
            material_.setBlue(v);
            emit materialChanged();
        }
        void setMetalness(float v) {
            material_.setMetalness(v);
            emit materialChanged();
        }
        void setRoughness(float v) {
            material_.setRoughness(v);
            emit materialChanged();
        }

        // Direct access
        Transform3D       &transform() { return transform_; }
        const Transform3D &transform() const { return transform_; }
        Material3D        &material() { return material_; }
        const Material3D  &material() const { return material_; }

        // Utility methods
        Q_INVOKABLE void resetTransform() {
            transform_.reset();
            emit transformChanged();
        }

        Q_INVOKABLE void setUniformScale(float s) {
            transform_.setUniformScale(s);
            emit transformChanged();
        }

        Q_INVOKABLE QColor getColor() const { return material_.color(); }

        Q_INVOKABLE void setColor(const QColor &c) {
            material_.setColor(c);
            emit materialChanged();
        }

    signals:
        void nameChanged();
        void visibleChanged();
        void transformChanged();
        void materialChanged();

    protected:
        std::string name_    = "Object";
        bool        visible_ = true;
        Transform3D transform_;
        Material3D  material_;
    };

    // ============================================================================
    // Primitive3D - Primitive shapes (cube, sphere, cylinder, etc.)
    // ============================================================================

    class Primitive3D : public SceneObject3D {
        Q_OBJECT
        Q_PROPERTY(PrimitiveType primitiveType READ primitiveType WRITE setPrimitiveType NOTIFY
                       primitiveTypeChanged)
        Q_PROPERTY(int segments READ segments WRITE setSegments NOTIFY segmentsChanged)
        Q_PROPERTY(int rings READ rings WRITE setRings NOTIFY ringsChanged)

    public:
        enum PrimitiveType { Cube, Sphere, Cylinder, Cone, Torus, Plane };
        Q_ENUM(PrimitiveType)

        explicit Primitive3D(QObject *parent = nullptr) : SceneObject3D(parent) {
            name_ = "Primitive";
        }

        PrimitiveType primitiveType() const { return primitive_type_; }
        void          setPrimitiveType(PrimitiveType t) {
            if (primitive_type_ != t) {
                primitive_type_ = t;
                emit primitiveTypeChanged();
            }
        }

        int  segments() const { return segments_; }
        void setSegments(int s) {
            if (segments_ != s) {
                segments_ = std::max(3, s);
                emit segmentsChanged();
            }
        }

        int  rings() const { return rings_; }
        void setRings(int r) {
            if (rings_ != r) {
                rings_ = std::max(2, r);
                emit ringsChanged();
            }
        }

    signals:
        void primitiveTypeChanged();
        void segmentsChanged();
        void ringsChanged();

    private:
        PrimitiveType primitive_type_ = Cube;
        int           segments_       = 32;
        int           rings_          = 16;
    };

    // ============================================================================
    // Light3D - Light sources
    // ============================================================================

    class Light3D : public SceneObject3D {
        Q_OBJECT
        Q_PROPERTY(LightType lightType READ lightType WRITE setLightType NOTIFY lightTypeChanged)
        Q_PROPERTY(float intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)
        Q_PROPERTY(float range READ range WRITE setRange NOTIFY rangeChanged)
        Q_PROPERTY(float spotAngle READ spotAngle WRITE setSpotAngle NOTIFY spotAngleChanged)
        Q_PROPERTY(bool castShadows READ castShadows WRITE setCastShadows NOTIFY castShadowsChanged)

    public:
        enum LightType { Directional, Point, Spot, Ambient };
        Q_ENUM(LightType)

        explicit Light3D(QObject *parent = nullptr) : SceneObject3D(parent) {
            name_ = "Light";
            material_.setRed(1.0f);
            material_.setGreen(1.0f);
            material_.setBlue(1.0f);
        }

        LightType lightType() const { return light_type_; }
        void      setLightType(LightType t) {
            if (light_type_ != t) {
                light_type_ = t;
                emit lightTypeChanged();
            }
        }

        float intensity() const { return intensity_; }
        void  setIntensity(float i) {
            if (intensity_ != i) {
                intensity_ = std::max(0.0f, i);
                emit intensityChanged();
            }
        }

        float range() const { return range_; }
        void  setRange(float r) {
            if (range_ != r) {
                range_ = std::max(0.1f, r);
                emit rangeChanged();
            }
        }

        float spotAngle() const { return spot_angle_; }
        void  setSpotAngle(float a) {
            if (spot_angle_ != a) {
                spot_angle_ = std::clamp(a, 1.0f, 179.0f);
                emit spotAngleChanged();
            }
        }

        bool castShadows() const { return cast_shadows_; }
        void setCastShadows(bool c) {
            if (cast_shadows_ != c) {
                cast_shadows_ = c;
                emit castShadowsChanged();
            }
        }

    signals:
        void lightTypeChanged();
        void intensityChanged();
        void rangeChanged();
        void spotAngleChanged();
        void castShadowsChanged();

    private:
        LightType light_type_   = Point;
        float     intensity_    = 1.0f;
        float     range_        = 10.0f;
        float     spot_angle_   = 45.0f;
        bool      cast_shadows_ = true;
    };

    // ============================================================================
    // Camera3D - Camera settings
    // ============================================================================

    class Camera3D : public SceneObject3D {
        Q_OBJECT
        Q_PROPERTY(float fieldOfView READ fieldOfView WRITE setFieldOfView NOTIFY fovChanged)
        Q_PROPERTY(float nearPlane READ nearPlane WRITE setNearPlane NOTIFY nearPlaneChanged)
        Q_PROPERTY(float farPlane READ farPlane WRITE setFarPlane NOTIFY farPlaneChanged)
        Q_PROPERTY(
            float aspectRatio READ aspectRatio WRITE setAspectRatio NOTIFY aspectRatioChanged)
        Q_PROPERTY(
            bool orthographic READ orthographic WRITE setOrthographic NOTIFY orthographicChanged)
        Q_PROPERTY(float orthoSize READ orthoSize WRITE setOrthoSize NOTIFY orthoSizeChanged)

    public:
        explicit Camera3D(QObject *parent = nullptr) : SceneObject3D(parent) {
            name_ = "Camera";
            transform_.setPositionZ(5.0f);
        }

        float fieldOfView() const { return fov_; }
        void  setFieldOfView(float f) {
            if (fov_ != f) {
                fov_ = std::clamp(f, 1.0f, 179.0f);
                emit fovChanged();
            }
        }

        float nearPlane() const { return near_plane_; }
        void  setNearPlane(float n) {
            if (near_plane_ != n) {
                near_plane_ = std::max(0.001f, n);
                emit nearPlaneChanged();
            }
        }

        float farPlane() const { return far_plane_; }
        void  setFarPlane(float f) {
            if (far_plane_ != f) {
                far_plane_ = std::max(near_plane_ + 0.1f, f);
                emit farPlaneChanged();
            }
        }

        float aspectRatio() const { return aspect_ratio_; }
        void  setAspectRatio(float a) {
            if (aspect_ratio_ != a) {
                aspect_ratio_ = std::max(0.1f, a);
                emit aspectRatioChanged();
            }
        }

        bool orthographic() const { return orthographic_; }
        void setOrthographic(bool o) {
            if (orthographic_ != o) {
                orthographic_ = o;
                emit orthographicChanged();
            }
        }

        float orthoSize() const { return ortho_size_; }
        void  setOrthoSize(float s) {
            if (ortho_size_ != s) {
                ortho_size_ = std::max(0.1f, s);
                emit orthoSizeChanged();
            }
        }

        Q_INVOKABLE void lookAt(float x, float y, float z) {
            // Simplified look-at implementation
            QVector3D pos = transform_.position();
            QVector3D target(x, y, z);
            QVector3D dir = (target - pos).normalized();

            float pitch = std::asin(-dir.y()) * 180.0f / M_PI;
            float yaw   = std::atan2(dir.x(), dir.z()) * 180.0f / M_PI;

            transform_.setRotationX(pitch);
            transform_.setRotationY(yaw);
            transform_.setRotationZ(0.0f);

            emit transformChanged();
        }

    signals:
        void fovChanged();
        void nearPlaneChanged();
        void farPlaneChanged();
        void aspectRatioChanged();
        void orthographicChanged();
        void orthoSizeChanged();

    private:
        float fov_          = 60.0f;
        float near_plane_   = 0.1f;
        float far_plane_    = 1000.0f;
        float aspect_ratio_ = 16.0f / 9.0f;
        bool  orthographic_ = false;
        float ortho_size_   = 5.0f;
    };

    // ============================================================================
    // Scene3D - Container for all 3D objects
    // ============================================================================

    class Scene3D : public QObject {
        Q_OBJECT
        Q_PROPERTY(
            QColor ambientColor READ ambientColor WRITE setAmbientColor NOTIFY ambientColorChanged)
        Q_PROPERTY(float ambientIntensity READ ambientIntensity WRITE setAmbientIntensity NOTIFY
                       ambientIntensityChanged)
        Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid NOTIFY showGridChanged)
        Q_PROPERTY(bool showAxes READ showAxes WRITE setShowAxes NOTIFY showAxesChanged)
        Q_PROPERTY(int objectCount READ objectCount NOTIFY objectCountChanged)

    public:
        explicit Scene3D(QObject *parent = nullptr) : QObject(parent) {
            // Add default camera and light
            camera_ = new Camera3D(this);
            camera_->setName("Main Camera");

            auto *light = new Light3D(this);
            light->setName("Sun");
            light->setLightType(Light3D::Directional);
            light->transform().setRotationX(-45.0f);
            light->transform().setRotationY(45.0f);
            objects_.push_back(light);
        }

        // Ambient lighting
        QColor ambientColor() const { return ambient_color_; }
        void   setAmbientColor(const QColor &c) {
            if (ambient_color_ != c) {
                ambient_color_ = c;
                emit ambientColorChanged();
            }
        }

        float ambientIntensity() const { return ambient_intensity_; }
        void  setAmbientIntensity(float i) {
            if (ambient_intensity_ != i) {
                ambient_intensity_ = std::clamp(i, 0.0f, 1.0f);
                emit ambientIntensityChanged();
            }
        }

        // Grid/Axes display
        bool showGrid() const { return show_grid_; }
        void setShowGrid(bool s) {
            if (show_grid_ != s) {
                show_grid_ = s;
                emit showGridChanged();
            }
        }

        bool showAxes() const { return show_axes_; }
        void setShowAxes(bool s) {
            if (show_axes_ != s) {
                show_axes_ = s;
                emit showAxesChanged();
            }
        }

        // Object management
        int objectCount() const { return static_cast<int>(objects_.size()); }

        Q_INVOKABLE Camera3D *camera() { return camera_; }

        Q_INVOKABLE SceneObject3D *objectAt(int index) {
            if (index >= 0 && index < static_cast<int>(objects_.size())) {
                return objects_[index];
            }
            return nullptr;
        }

        Q_INVOKABLE Primitive3D *addPrimitive(int type = 0) {
            auto *prim = new Primitive3D(this);
            prim->setPrimitiveType(static_cast<Primitive3D::PrimitiveType>(type));
            prim->setName(QString("Primitive_%1").arg(objects_.size()));
            objects_.push_back(prim);
            emit objectCountChanged();
            emit objectAdded(prim);
            return prim;
        }

        Q_INVOKABLE Light3D *addLight(int type = 1) {
            auto *light = new Light3D(this);
            light->setLightType(static_cast<Light3D::LightType>(type));
            light->setName(QString("Light_%1").arg(objects_.size()));
            objects_.push_back(light);
            emit objectCountChanged();
            emit objectAdded(light);
            return light;
        }

        Q_INVOKABLE void removeObject(int index) {
            if (index >= 0 && index < static_cast<int>(objects_.size())) {
                auto *obj = objects_[index];
                objects_.erase(objects_.begin() + index);
                emit objectCountChanged();
                emit objectRemoved(obj);
                obj->deleteLater();
            }
        }

        Q_INVOKABLE void clearObjects() {
            for (auto *obj : objects_) {
                emit objectRemoved(obj);
                obj->deleteLater();
            }
            objects_.clear();
            emit objectCountChanged();
        }

        Q_INVOKABLE QStringList objectNames() const {
            QStringList names;
            for (const auto *obj : objects_) {
                names.append(obj->name());
            }
            return names;
        }

    signals:
        void ambientColorChanged();
        void ambientIntensityChanged();
        void showGridChanged();
        void showAxesChanged();
        void objectCountChanged();
        void objectAdded(SceneObject3D *obj);
        void objectRemoved(SceneObject3D *obj);

    private:
        Camera3D                    *camera_ = nullptr;
        std::vector<SceneObject3D *> objects_;
        QColor                       ambient_color_     = QColor(40, 40, 50);
        float                        ambient_intensity_ = 0.3f;
        bool                         show_grid_         = true;
        bool                         show_axes_         = true;
    };

    // ============================================================================
    // QML Registration
    // ============================================================================

    inline void registerQmlTypes() {
        qmlRegisterType<SceneObject3D>("Rosetta3D", 1, 0, "SceneObject3D");
        qmlRegisterType<Primitive3D>("Rosetta3D", 1, 0, "Primitive3D");
        qmlRegisterType<Light3D>("Rosetta3D", 1, 0, "Light3D");
        qmlRegisterType<Camera3D>("Rosetta3D", 1, 0, "Camera3D");
        qmlRegisterType<Scene3D>("Rosetta3D", 1, 0, "Scene3D");
    }

} // namespace rosetta::qt3d
