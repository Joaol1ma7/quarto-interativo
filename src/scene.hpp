#pragma once
#include "math.hpp"
#include <cstdint>
#include <string>
#include <vector>

constexpr float roomWidth = 8.5f, roomDepth = 4.75f, roomHeight = 2.8f;
constexpr float recessWidth = 0.9f, recessStart = 3.28f;
constexpr float windowZMin=1.85f, windowZMax=3.12f;
constexpr float windowBottom=.85f, windowTop=2.10f;
constexpr float doorZMin=3.60f, doorZMax=4.55f, doorTop=2.20f;
constexpr float cameraRadius = 0.18f;
constexpr Vec3 initialCamera{7.15f,1.65f,3.70f};
constexpr Vec3 initialTarget{3.0f,.95f,1.6f};

struct Vec2 { float x, y; };
struct Vertex { Vec3 position, normal, color; Vec2 uv; };
using MeshData = std::vector<Vertex>;
enum class MeshKind { Cube, Sphere, Cylinder, Shade, Shell, Floor, Count };
enum class TextureKind { White, Floor, Wood, Ball, Art, Screen, Count };
enum class MaterialId {
    Wall, Floor, Wood, DarkWood, Fabric, ChairFabric, WhiteFabric, Metal, Plastic,
    Glass, Screen, Art, Ball, Light, KeyPlastic,
    Grass, HillFar, HillNear, Leaves, Bark, Path, Cloud, Sun, Moon, Star, Count
};
struct Material {
    Vec3 color;
    float specular, shininess, alpha, emission;
    TextureKind texture;
};

// Cada peca conserva sua matriz local. As partes moveis compartilham um pivo.
enum class Group { Static, Door, Window, Ball, Sun };
struct Part {
    std::string object;
    MeshKind mesh;
    MaterialId material;
    Mat4 local;
    Group group = Group::Static;
    bool outdoor = false;
    unsigned char periodMask = 7; // bit 0: manha; bit 1: tarde; bit 2: noite
};
struct Collider { Vec2 center, halfSize; float yaw = 0; };
enum class DayPeriod { Morning, Afternoon, Night };
struct Scene {
    std::vector<Part> parts;
    std::vector<Collider> colliders;
    float doorAngle = 0, windowAngle = 0;
    Vec3 ballPosition{2.4f, 0.20f, 2.28f};
    float ballRotation = 0;
    Mat4 ballOrientation = identity();
    Vec3 ballVelocity{0,0,0};
    bool ballNear = false;
    bool doorOpen = false, windowOpen = false, lampOn = true;
    bool doorBlocked = false, windowBlocked = false;
    DayPeriod period = DayPeriod::Morning;
};

MeshData makeMesh(MeshKind kind);
std::vector<Material> makeMaterials();
std::vector<std::uint8_t> makeTexture(TextureKind kind, int size);
Scene makeScene();
void addLandscape(Scene& scene);
Mat4 partModel(const Scene& scene, const Part& part);
inline bool visiblePart(const Scene& scene, const Part& part) {
    return (part.periodMask & (1u << static_cast<int>(scene.period))) != 0;
}
bool canWalk(const Scene& scene, float x, float z);
