// ============================================================================
// Rosetta ImGui Property Editor - 3D Scene Editor Example
// ============================================================================
//
// A complete example showing:
// - 3D OpenGL rendering with camera controls
// - Scene hierarchy with selectable objects
// - Real-time property editing via Rosetta introspection
// - Transform, Material, and Light components
//
// ============================================================================

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <rosetta/rosetta.h>
#include <rosetta/extensions/gui/imgui/imgui_property_editor.h>

#include <array>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

// ============================================================================
// Math Utilities
// ============================================================================

struct Vec3 {
    float x = 0, y = 0, z = 0;
    
    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vec3 operator-(const Vec3& v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3 normalized() const { 
        float len = length();
        return len > 0 ? Vec3{x/len, y/len, z/len} : Vec3{0,0,0};
    }
    
    static Vec3 cross(const Vec3& a, const Vec3& b) {
        return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
    }
    
    static float dot(const Vec3& a, const Vec3& b) {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }
};

struct Color3 {
    float r = 1, g = 1, b = 1;
    
    Color3() = default;
    Color3(float r, float g, float b) : r(r), g(g), b(b) {}
};

struct Color4 {
    float r = 1, g = 1, b = 1, a = 1;
    
    Color4() = default;
    Color4(float r, float g, float b, float a = 1) : r(r), g(g), b(b), a(a) {}
};

// ============================================================================
// Components
// ============================================================================

struct Transform {
    Vec3 position{0, 0, 0};
    Vec3 rotation{0, 0, 0};  // Euler angles in degrees
    Vec3 scale{1, 1, 1};
    
    void reset() {
        position = {0, 0, 0};
        rotation = {0, 0, 0};
        scale = {1, 1, 1};
    }
    
    void translate(float dx, float dy, float dz) {
        position.x += dx;
        position.y += dy;
        position.z += dz;
    }
    
    void rotate(float rx, float ry, float rz) {
        rotation.x += rx;
        rotation.y += ry;
        rotation.z += rz;
    }
};

struct Material {
    Color3 color{1, 1, 1};
    Color3 emission{0, 0, 0};
    float ambient = 0.2f;
    float diffuse = 0.8f;
    float specular = 0.5f;
    float shininess = 32.0f;
    bool wireframe = false;
    
    void set_red() { color = {1, 0.2f, 0.2f}; }
    void set_green() { color = {0.2f, 1, 0.2f}; }
    void set_blue() { color = {0.2f, 0.2f, 1}; }
    void set_gold() { color = {1, 0.84f, 0}; specular = 0.9f; shininess = 64; }
};

enum class PrimitiveType {
    Cube,
    Sphere,
    Cylinder,
    Cone,
    Plane
};

struct SceneObject {
    std::string name = "Object";
    bool visible = true;
    bool selected = false;
    PrimitiveType type = PrimitiveType::Cube;
    Transform transform;
    Material material;
    
    void show() { visible = true; }
    void hide() { visible = false; }
    void reset_transform() { transform.reset(); }
};

struct PointLight {
    std::string name = "Light";
    bool enabled = true;
    Vec3 position{5, 5, 5};
    Color3 color{1, 1, 1};
    float intensity = 1.0f;
    float range = 50.0f;
    
    void set_warm() { color = {1, 0.9f, 0.7f}; }
    void set_cool() { color = {0.7f, 0.85f, 1}; }
    void set_position(float x, float y, float z) { position = {x, y, z}; }
};

struct Camera {
    Vec3 position{0, 3, 8};
    Vec3 target{0, 0, 0};
    float fov = 60.0f;
    float near_plane = 0.1f;
    float far_plane = 100.0f;
    
    // Orbit camera state
    float orbit_distance = 8.0f;
    float orbit_yaw = 0.0f;      // Horizontal angle in degrees
    float orbit_pitch = 20.0f;   // Vertical angle in degrees
    
    void update_from_orbit() {
        float yaw_rad = orbit_yaw * 3.14159f / 180.0f;
        float pitch_rad = orbit_pitch * 3.14159f / 180.0f;
        
        position.x = target.x + orbit_distance * std::cos(pitch_rad) * std::sin(yaw_rad);
        position.y = target.y + orbit_distance * std::sin(pitch_rad);
        position.z = target.z + orbit_distance * std::cos(pitch_rad) * std::cos(yaw_rad);
    }
    
    void orbit(float dyaw, float dpitch) {
        orbit_yaw += dyaw;
        orbit_pitch += dpitch;
        orbit_pitch = std::max(-89.0f, std::min(89.0f, orbit_pitch));
        update_from_orbit();
    }
    
    void zoom(float delta) {
        orbit_distance -= delta;
        orbit_distance = std::max(1.0f, std::min(50.0f, orbit_distance));
        update_from_orbit();
    }
    
    void reset() {
        orbit_distance = 8.0f;
        orbit_yaw = 0.0f;
        orbit_pitch = 20.0f;
        target = {0, 0, 0};
        update_from_orbit();
    }
};

// ============================================================================
// Scene
// ============================================================================

class Scene {
public:
    std::vector<std::unique_ptr<SceneObject>> objects;
    std::vector<std::unique_ptr<PointLight>> lights;
    Camera camera;
    Color3 ambient_color{0.1f, 0.1f, 0.15f};
    Color3 clear_color{0.15f, 0.15f, 0.2f};
    bool show_grid = true;
    bool show_axes = true;
    
    Scene() {
        camera.update_from_orbit();
        create_default_scene();
    }
    
    void create_default_scene() {
        // Ground plane
        auto ground = std::make_unique<SceneObject>();
        ground->name = "Ground";
        ground->type = PrimitiveType::Plane;
        ground->transform.scale = {10, 1, 10};
        ground->material.color = {0.3f, 0.35f, 0.3f};
        ground->material.specular = 0.1f;
        objects.push_back(std::move(ground));
        
        // Red cube
        auto cube = std::make_unique<SceneObject>();
        cube->name = "Red Cube";
        cube->type = PrimitiveType::Cube;
        cube->transform.position = {-2, 0.5f, 0};
        cube->material.color = {0.9f, 0.2f, 0.2f};
        objects.push_back(std::move(cube));
        
        // Green sphere
        auto sphere = std::make_unique<SceneObject>();
        sphere->name = "Green Sphere";
        sphere->type = PrimitiveType::Sphere;
        sphere->transform.position = {0, 0.75f, 0};
        sphere->transform.scale = {0.75f, 0.75f, 0.75f};
        sphere->material.color = {0.2f, 0.8f, 0.3f};
        sphere->material.shininess = 64;
        objects.push_back(std::move(sphere));
        
        // Blue cylinder
        auto cylinder = std::make_unique<SceneObject>();
        cylinder->name = "Blue Cylinder";
        cylinder->type = PrimitiveType::Cylinder;
        cylinder->transform.position = {2, 0.75f, 0};
        cylinder->transform.scale = {0.5f, 0.75f, 0.5f};
        cylinder->material.color = {0.2f, 0.4f, 0.9f};
        objects.push_back(std::move(cylinder));
        
        // Gold cone
        auto cone = std::make_unique<SceneObject>();
        cone->name = "Gold Cone";
        cone->type = PrimitiveType::Cone;
        cone->transform.position = {0, 0.5f, 2};
        cone->transform.scale = {0.6f, 1.0f, 0.6f};
        cone->material.set_gold();
        objects.push_back(std::move(cone));
        
        // Main light
        auto light1 = std::make_unique<PointLight>();
        light1->name = "Main Light";
        light1->position = {3, 5, 3};
        light1->color = {1, 0.95f, 0.9f};
        light1->intensity = 1.0f;
        lights.push_back(std::move(light1));
        
        // Fill light
        auto light2 = std::make_unique<PointLight>();
        light2->name = "Fill Light";
        light2->position = {-4, 3, -2};
        light2->color = {0.6f, 0.7f, 1.0f};
        light2->intensity = 0.5f;
        lights.push_back(std::move(light2));
    }
    
    SceneObject* add_object(PrimitiveType type, const std::string& name) {
        auto obj = std::make_unique<SceneObject>();
        obj->name = name;
        obj->type = type;
        obj->transform.position.y = 0.5f;
        objects.push_back(std::move(obj));
        return objects.back().get();
    }
    
    PointLight* add_light(const std::string& name) {
        auto light = std::make_unique<PointLight>();
        light->name = name;
        lights.push_back(std::move(light));
        return lights.back().get();
    }
    
    void clear() {
        objects.clear();
        lights.clear();
    }
};

// ============================================================================
// OpenGL Renderer
// ============================================================================

class Renderer {
public:
    void init() {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    
    void render(Scene& scene, int viewport_width, int viewport_height) {
        // Clear
        glClearColor(scene.clear_color.r, scene.clear_color.g, scene.clear_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Setup matrices
        setup_projection(scene.camera, viewport_width, viewport_height);
        setup_view(scene.camera);
        
        // Draw grid
        if (scene.show_grid) {
            draw_grid(20, 1.0f);
        }
        
        // Draw axes
        if (scene.show_axes) {
            draw_axes();
        }
        
        // Draw lights (as small spheres)
        for (auto& light : scene.lights) {
            if (light->enabled) {
                draw_light_gizmo(*light);
            }
        }
        
        // Calculate simple lighting
        Vec3 light_pos{0, 5, 5};
        Color3 light_color{1, 1, 1};
        float light_intensity = 1.0f;
        
        if (!scene.lights.empty() && scene.lights[0]->enabled) {
            light_pos = scene.lights[0]->position;
            light_color = scene.lights[0]->color;
            light_intensity = scene.lights[0]->intensity;
        }
        
        // Draw objects
        for (auto& obj : scene.objects) {
            if (obj->visible) {
                draw_object(*obj, scene.camera.position, light_pos, light_color, 
                           light_intensity, scene.ambient_color);
            }
        }
    }
    
private:
    void setup_projection(const Camera& cam, int w, int h) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
        float aspect = (float)w / (float)h;
        float fov_rad = cam.fov * 3.14159f / 180.0f;
        float f = 1.0f / std::tan(fov_rad / 2.0f);
        
        float proj[16] = {
            f / aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, (cam.far_plane + cam.near_plane) / (cam.near_plane - cam.far_plane), -1,
            0, 0, (2 * cam.far_plane * cam.near_plane) / (cam.near_plane - cam.far_plane), 0
        };
        glLoadMatrixf(proj);
    }
    
    void setup_view(const Camera& cam) {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        Vec3 forward = (cam.target - cam.position).normalized();
        Vec3 right = Vec3::cross(forward, {0, 1, 0}).normalized();
        Vec3 up = Vec3::cross(right, forward);
        
        float view[16] = {
            right.x, up.x, -forward.x, 0,
            right.y, up.y, -forward.y, 0,
            right.z, up.z, -forward.z, 0,
            -Vec3::dot(right, cam.position), -Vec3::dot(up, cam.position), Vec3::dot(forward, cam.position), 1
        };
        glLoadMatrixf(view);
    }
    
    void apply_transform(const Transform& t) {
        glTranslatef(t.position.x, t.position.y, t.position.z);
        glRotatef(t.rotation.y, 0, 1, 0);
        glRotatef(t.rotation.x, 1, 0, 0);
        glRotatef(t.rotation.z, 0, 0, 1);
        glScalef(t.scale.x, t.scale.y, t.scale.z);
    }
    
    void draw_grid(int size, float spacing) {
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        glColor3f(0.3f, 0.3f, 0.3f);
        
        for (int i = -size; i <= size; i++) {
            float pos = i * spacing;
            glVertex3f(pos, 0, -size * spacing);
            glVertex3f(pos, 0, size * spacing);
            glVertex3f(-size * spacing, 0, pos);
            glVertex3f(size * spacing, 0, pos);
        }
        glEnd();
    }
    
    void draw_axes() {
        glDisable(GL_LIGHTING);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        
        // X axis - red
        glColor3f(1, 0.2f, 0.2f);
        glVertex3f(0, 0.01f, 0);
        glVertex3f(3, 0.01f, 0);
        
        // Y axis - green
        glColor3f(0.2f, 1, 0.2f);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 3, 0);
        
        // Z axis - blue
        glColor3f(0.2f, 0.2f, 1);
        glVertex3f(0, 0.01f, 0);
        glVertex3f(0, 0.01f, 3);
        
        glEnd();
        glLineWidth(1.0f);
    }
    
    void draw_light_gizmo(const PointLight& light) {
        glPushMatrix();
        glTranslatef(light.position.x, light.position.y, light.position.z);
        
        glDisable(GL_LIGHTING);
        glColor3f(light.color.r, light.color.g, light.color.b);
        
        // Draw a small diamond shape
        float s = 0.15f;
        glBegin(GL_LINES);
        glVertex3f(-s, 0, 0); glVertex3f(s, 0, 0);
        glVertex3f(0, -s, 0); glVertex3f(0, s, 0);
        glVertex3f(0, 0, -s); glVertex3f(0, 0, s);
        
        glVertex3f(0, s, 0); glVertex3f(s, 0, 0);
        glVertex3f(0, s, 0); glVertex3f(-s, 0, 0);
        glVertex3f(0, s, 0); glVertex3f(0, 0, s);
        glVertex3f(0, s, 0); glVertex3f(0, 0, -s);
        
        glVertex3f(0, -s, 0); glVertex3f(s, 0, 0);
        glVertex3f(0, -s, 0); glVertex3f(-s, 0, 0);
        glVertex3f(0, -s, 0); glVertex3f(0, 0, s);
        glVertex3f(0, -s, 0); glVertex3f(0, 0, -s);
        glEnd();
        
        glPopMatrix();
    }
    
    void draw_object(SceneObject& obj, const Vec3& cam_pos, const Vec3& light_pos,
                     const Color3& light_color, float light_intensity,
                     const Color3& ambient) {
        glPushMatrix();
        apply_transform(obj.transform);
        
        if (obj.material.wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        
        // Selection highlight
        if (obj.selected) {
            glDisable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(2.0f);
            glColor3f(1, 0.8f, 0);
            draw_primitive(obj.type);
            glLineWidth(1.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
        }
        
        // Set color with simple lighting calculation
        glColor3f(
            obj.material.color.r * (ambient.r + light_intensity * light_color.r * obj.material.diffuse),
            obj.material.color.g * (ambient.g + light_intensity * light_color.g * obj.material.diffuse),
            obj.material.color.b * (ambient.b + light_intensity * light_color.b * obj.material.diffuse)
        );
        
        draw_primitive(obj.type);
        
        if (obj.material.wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
        glPopMatrix();
    }
    
    void draw_primitive(PrimitiveType type) {
        switch (type) {
            case PrimitiveType::Cube: draw_cube(); break;
            case PrimitiveType::Sphere: draw_sphere(16, 12); break;
            case PrimitiveType::Cylinder: draw_cylinder(16); break;
            case PrimitiveType::Cone: draw_cone(16); break;
            case PrimitiveType::Plane: draw_plane(); break;
        }
    }
    
    void draw_cube() {
        glBegin(GL_QUADS);
        // Front
        glNormal3f(0, 0, 1);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        // Back
        glNormal3f(0, 0, -1);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        // Top
        glNormal3f(0, 1, 0);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        // Bottom
        glNormal3f(0, -1, 0);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        // Right
        glNormal3f(1, 0, 0);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        // Left
        glNormal3f(-1, 0, 0);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glEnd();
    }
    
    void draw_sphere(int slices, int stacks) {
        for (int i = 0; i < stacks; ++i) {
            float lat0 = 3.14159f * (-0.5f + (float)i / stacks);
            float lat1 = 3.14159f * (-0.5f + (float)(i + 1) / stacks);
            float y0 = std::sin(lat0) * 0.5f;
            float y1 = std::sin(lat1) * 0.5f;
            float r0 = std::cos(lat0) * 0.5f;
            float r1 = std::cos(lat1) * 0.5f;
            
            glBegin(GL_QUAD_STRIP);
            for (int j = 0; j <= slices; ++j) {
                float lng = 2 * 3.14159f * (float)j / slices;
                float x = std::cos(lng);
                float z = std::sin(lng);
                
                glNormal3f(x * r0 * 2, y0 * 2, z * r0 * 2);
                glVertex3f(x * r0, y0, z * r0);
                glNormal3f(x * r1 * 2, y1 * 2, z * r1 * 2);
                glVertex3f(x * r1, y1, z * r1);
            }
            glEnd();
        }
    }
    
    void draw_cylinder(int segments) {
        float radius = 0.5f;
        float height = 1.0f;
        
        // Side
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= segments; ++i) {
            float angle = 2 * 3.14159f * i / segments;
            float x = std::cos(angle) * radius;
            float z = std::sin(angle) * radius;
            glNormal3f(std::cos(angle), 0, std::sin(angle));
            glVertex3f(x, -height/2, z);
            glVertex3f(x, height/2, z);
        }
        glEnd();
        
        // Top cap
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 1, 0);
        glVertex3f(0, height/2, 0);
        for (int i = 0; i <= segments; ++i) {
            float angle = 2 * 3.14159f * i / segments;
            glVertex3f(std::cos(angle) * radius, height/2, std::sin(angle) * radius);
        }
        glEnd();
        
        // Bottom cap
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, -1, 0);
        glVertex3f(0, -height/2, 0);
        for (int i = segments; i >= 0; --i) {
            float angle = 2 * 3.14159f * i / segments;
            glVertex3f(std::cos(angle) * radius, -height/2, std::sin(angle) * radius);
        }
        glEnd();
    }
    
    void draw_cone(int segments) {
        float radius = 0.5f;
        float height = 1.0f;
        
        // Side
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, height/2, 0);
        for (int i = 0; i <= segments; ++i) {
            float angle = 2 * 3.14159f * i / segments;
            float x = std::cos(angle) * radius;
            float z = std::sin(angle) * radius;
            glNormal3f(std::cos(angle), 0.5f, std::sin(angle));
            glVertex3f(x, -height/2, z);
        }
        glEnd();
        
        // Bottom cap
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, -1, 0);
        glVertex3f(0, -height/2, 0);
        for (int i = segments; i >= 0; --i) {
            float angle = 2 * 3.14159f * i / segments;
            glVertex3f(std::cos(angle) * radius, -height/2, std::sin(angle) * radius);
        }
        glEnd();
    }
    
    void draw_plane() {
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(-0.5f, 0, -0.5f);
        glVertex3f(-0.5f, 0, 0.5f);
        glVertex3f(0.5f, 0, 0.5f);
        glVertex3f(0.5f, 0, -0.5f);
        glEnd();
    }
};

// ============================================================================
// Rosetta Registration
// ============================================================================

void register_types() {
    using namespace rosetta::core;
    
    // Vec3
    Registry::instance()
        .register_class<Vec3>("Vec3")
        .field("x", &Vec3::x)
        .field("y", &Vec3::y)
        .field("z", &Vec3::z);
    
    // Color3
    Registry::instance()
        .register_class<Color3>("Color3")
        .field("r", &Color3::r)
        .field("g", &Color3::g)
        .field("b", &Color3::b);
    
    // Transform
    Registry::instance()
        .register_class<Transform>("Transform")
        .field("position", &Transform::position)
        .field("rotation", &Transform::rotation)
        .field("scale", &Transform::scale)
        .method("reset", &Transform::reset)
        .method("translate", &Transform::translate)
        .method("rotate", &Transform::rotate);
    
    // Material
    Registry::instance()
        .register_class<Material>("Material")
        .field("color", &Material::color)
        .field("emission", &Material::emission)
        .field("ambient", &Material::ambient)
        .field("diffuse", &Material::diffuse)
        .field("specular", &Material::specular)
        .field("shininess", &Material::shininess)
        .field("wireframe", &Material::wireframe)
        .method("set_red", &Material::set_red)
        .method("set_green", &Material::set_green)
        .method("set_blue", &Material::set_blue)
        .method("set_gold", &Material::set_gold);
    
    // SceneObject
    Registry::instance()
        .register_class<SceneObject>("SceneObject")
        .field("name", &SceneObject::name)
        .field("visible", &SceneObject::visible)
        .field("transform", &SceneObject::transform)
        .field("material", &SceneObject::material)
        .method("show", &SceneObject::show)
        .method("hide", &SceneObject::hide)
        .method("reset_transform", &SceneObject::reset_transform);
    
    // PointLight
    Registry::instance()
        .register_class<PointLight>("PointLight")
        .field("name", &PointLight::name)
        .field("enabled", &PointLight::enabled)
        .field("position", &PointLight::position)
        .field("color", &PointLight::color)
        .field("intensity", &PointLight::intensity)
        .field("range", &PointLight::range)
        .method("set_warm", &PointLight::set_warm)
        .method("set_cool", &PointLight::set_cool)
        .method("set_position", &PointLight::set_position);
    
    // Camera
    Registry::instance()
        .register_class<Camera>("Camera")
        .field("position", &Camera::position)
        .field("target", &Camera::target)
        .field("fov", &Camera::fov)
        .field("near_plane", &Camera::near_plane)
        .field("far_plane", &Camera::far_plane)
        .field("orbit_distance", &Camera::orbit_distance)
        .field("orbit_yaw", &Camera::orbit_yaw)
        .field("orbit_pitch", &Camera::orbit_pitch)
        .method("reset", &Camera::reset)
        .method("zoom", &Camera::zoom)
        .method("orbit", &Camera::orbit);
}

// Register custom widget for Vec3
void register_vec3_widget() {
    using namespace rosetta::imgui;
    using namespace rosetta::core;
    
    WidgetDrawer::instance().register_drawer(
        std::type_index(typeid(Vec3)),
        [](const std::string& label, std::function<Any()> getter,
           std::function<void(const Any&)> setter, 
           PropertyChangedCallback on_change) -> bool {
            Vec3 v = getter().as<Vec3>();
            float vec[3] = {v.x, v.y, v.z};
            
            if (ImGui::DragFloat3(label.c_str(), vec, 0.05f)) {
                setter(Any(Vec3{vec[0], vec[1], vec[2]}));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        }
    );
}

// Register custom widget for Color3
void register_color3_widget() {
    using namespace rosetta::imgui;
    using namespace rosetta::core;
    
    WidgetDrawer::instance().register_drawer(
        std::type_index(typeid(Color3)),
        [](const std::string& label, std::function<Any()> getter,
           std::function<void(const Any&)> setter,
           PropertyChangedCallback on_change) -> bool {
            Color3 c = getter().as<Color3>();
            float col[3] = {c.r, c.g, c.b};
            
            if (ImGui::ColorEdit3(label.c_str(), col)) {
                setter(Any(Color3{col[0], col[1], col[2]}));
                if (on_change) on_change(label);
                return true;
            }
            return false;
        }
    );
}

// ============================================================================
// Application
// ============================================================================

class Application {
public:
    Scene scene;
    Renderer renderer;
    
    // Selection state
    int selected_object_index = -1;
    int selected_light_index = -1;
    enum class SelectionType { None, Object, Light, Camera } selection_type = SelectionType::None;
    
    // Property editors
    std::unique_ptr<rosetta::imgui::ObjectInspector<SceneObject>> object_inspector;
    std::unique_ptr<rosetta::imgui::ObjectInspector<PointLight>> light_inspector;
    std::unique_ptr<rosetta::imgui::PropertyEditor<Camera>> camera_editor;
    
    // Camera control state
    bool mouse_captured = false;
    double last_mouse_x = 0, last_mouse_y = 0;
    
    void init() {
        renderer.init();
        
        // Create property editors
        camera_editor = std::make_unique<rosetta::imgui::PropertyEditor<Camera>>(&scene.camera);
        camera_editor->set_on_change([this](const std::string&) {
            scene.camera.update_from_orbit();
        });
    }
    
    void update(GLFWwindow* window) {
        handle_camera_input(window);
    }
    
    void handle_camera_input(GLFWwindow* window) {
        // Only handle camera when not over ImGui
        if (ImGui::GetIO().WantCaptureMouse) {
            mouse_captured = false;
            return;
        }
        
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        
        // Right mouse button for orbit
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            if (!mouse_captured) {
                mouse_captured = true;
                last_mouse_x = mx;
                last_mouse_y = my;
            } else {
                float dx = (float)(mx - last_mouse_x);
                float dy = (float)(my - last_mouse_y);
                scene.camera.orbit(-dx * 0.3f, -dy * 0.3f);
                last_mouse_x = mx;
                last_mouse_y = my;
            }
        } else {
            mouse_captured = false;
        }
        
        // Scroll for zoom
        // Note: scroll callback should be set up for proper zoom
    }
    
    void render(int width, int height) {
        renderer.render(scene, width, height);
    }
    
    void draw_ui() {
        draw_hierarchy_panel();
        draw_inspector_panel();
        draw_scene_settings_panel();
    }
    
    void draw_hierarchy_panel() {
        ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_FirstUseEver);
        
        ImGui::Begin("Hierarchy");
        
        // Objects section
        if (ImGui::CollapsingHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (size_t i = 0; i < scene.objects.size(); ++i) {
                auto& obj = scene.objects[i];
                
                ImGui::PushID(static_cast<int>(i));
                
                // Visibility toggle
                bool visible = obj->visible;
                if (ImGui::Checkbox("##vis", &visible)) {
                    obj->visible = visible;
                }
                ImGui::SameLine();
                
                // Selection
                bool is_selected = (selection_type == SelectionType::Object && 
                                   selected_object_index == static_cast<int>(i));
                if (ImGui::Selectable(obj->name.c_str(), is_selected)) {
                    select_object(static_cast<int>(i));
                }
                
                ImGui::PopID();
            }
            
            ImGui::Spacing();
            if (ImGui::Button("+ Add Cube")) {
                scene.add_object(PrimitiveType::Cube, "New Cube");
            }
            ImGui::SameLine();
            if (ImGui::Button("+ Sphere")) {
                scene.add_object(PrimitiveType::Sphere, "New Sphere");
            }
        }
        
        // Lights section
        if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (size_t i = 0; i < scene.lights.size(); ++i) {
                auto& light = scene.lights[i];
                
                ImGui::PushID(static_cast<int>(1000 + i));
                
                bool enabled = light->enabled;
                if (ImGui::Checkbox("##en", &enabled)) {
                    light->enabled = enabled;
                }
                ImGui::SameLine();
                
                bool is_selected = (selection_type == SelectionType::Light && 
                                   selected_light_index == static_cast<int>(i));
                if (ImGui::Selectable(light->name.c_str(), is_selected)) {
                    select_light(static_cast<int>(i));
                }
                
                ImGui::PopID();
            }
            
            ImGui::Spacing();
            if (ImGui::Button("+ Add Light")) {
                scene.add_light("New Light");
            }
        }
        
        // Camera
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool is_selected = (selection_type == SelectionType::Camera);
            if (ImGui::Selectable("Main Camera", is_selected)) {
                select_camera();
            }
        }
        
        ImGui::End();
    }
    
    void draw_inspector_panel() {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 320, 30), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(310, 500), ImGuiCond_FirstUseEver);
        
        ImGui::Begin("Inspector");
        
        switch (selection_type) {
            case SelectionType::Object:
                if (selected_object_index >= 0 && 
                    selected_object_index < static_cast<int>(scene.objects.size())) {
                    if (object_inspector) {
                        object_inspector->draw();
                    }
                }
                break;
                
            case SelectionType::Light:
                if (selected_light_index >= 0 && 
                    selected_light_index < static_cast<int>(scene.lights.size())) {
                    if (light_inspector) {
                        light_inspector->draw();
                    }
                }
                break;
                
            case SelectionType::Camera:
                ImGui::Text("Camera");
                ImGui::Separator();
                if (camera_editor) {
                    camera_editor->draw_fields();
                }
                ImGui::Spacing();
                if (ImGui::Button("Reset Camera")) {
                    scene.camera.reset();
                }
                break;
                
            default:
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
                                  "Select an object to edit");
                break;
        }
        
        ImGui::End();
    }
    
    void draw_scene_settings_panel() {
        ImGui::SetNextWindowPos(ImVec2(10, 450), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(200, 150), ImGuiCond_FirstUseEver);
        
        ImGui::Begin("Scene Settings");
        
        ImGui::Checkbox("Show Grid", &scene.show_grid);
        ImGui::Checkbox("Show Axes", &scene.show_axes);
        
        float ambient[3] = {scene.ambient_color.r, scene.ambient_color.g, scene.ambient_color.b};
        if (ImGui::ColorEdit3("Ambient", ambient)) {
            scene.ambient_color = {ambient[0], ambient[1], ambient[2]};
        }
        
        float clear[3] = {scene.clear_color.r, scene.clear_color.g, scene.clear_color.b};
        if (ImGui::ColorEdit3("Background", clear)) {
            scene.clear_color = {clear[0], clear[1], clear[2]};
        }
        
        ImGui::End();
    }
    
    void select_object(int index) {
        // Deselect previous
        if (selection_type == SelectionType::Object && selected_object_index >= 0 &&
            selected_object_index < static_cast<int>(scene.objects.size())) {
            scene.objects[selected_object_index]->selected = false;
        }
        
        selection_type = SelectionType::Object;
        selected_object_index = index;
        selected_light_index = -1;
        
        if (index >= 0 && index < static_cast<int>(scene.objects.size())) {
            scene.objects[index]->selected = true;
            object_inspector = std::make_unique<rosetta::imgui::ObjectInspector<SceneObject>>(
                scene.objects[index].get());
        }
    }
    
    void select_light(int index) {
        // Deselect previous object
        if (selection_type == SelectionType::Object && selected_object_index >= 0 &&
            selected_object_index < static_cast<int>(scene.objects.size())) {
            scene.objects[selected_object_index]->selected = false;
        }
        
        selection_type = SelectionType::Light;
        selected_object_index = -1;
        selected_light_index = index;
        
        if (index >= 0 && index < static_cast<int>(scene.lights.size())) {
            light_inspector = std::make_unique<rosetta::imgui::ObjectInspector<PointLight>>(
                scene.lights[index].get());
        }
    }
    
    void select_camera() {
        // Deselect previous object
        if (selection_type == SelectionType::Object && selected_object_index >= 0 &&
            selected_object_index < static_cast<int>(scene.objects.size())) {
            scene.objects[selected_object_index]->selected = false;
        }
        
        selection_type = SelectionType::Camera;
        selected_object_index = -1;
        selected_light_index = -1;
    }
};

// ============================================================================
// Main
// ============================================================================

static Application* g_app = nullptr;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;
    if (!ImGui::GetIO().WantCaptureMouse && g_app) {
        g_app->scene.camera.zoom((float)yoffset * 0.5f);
    }
}

void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    // Register Rosetta types
    register_types();
    register_vec3_widget();
    register_color3_widget();
    
    // Initialize GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }
    
    // OpenGL hints
#if defined(__APPLE__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    const char* glsl_version = "#version 120";
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    const char* glsl_version = "#version 120";
#endif
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(1400, 900, 
        "Rosetta 3D Scene Editor", nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetScrollCallback(window, scroll_callback);
    
    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    (void)io;
    
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    // Create application
    Application app;
    g_app = &app;
    app.init();
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Get window size
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        
        // Update
        app.update(window);
        
        // Render 3D scene
        glViewport(0, 0, display_w, display_h);
        app.render(display_w, display_h);
        
        // Render ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        app.draw_ui();
        
        // Help text
        ImGui::SetNextWindowPos(ImVec2(220, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::Begin("Controls", nullptr, 
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | 
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::Text("Right-click + drag: Orbit camera");
        ImGui::Text("Scroll: Zoom");
        ImGui::End();
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    
    // Cleanup
    g_app = nullptr;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}