#pragma once
#include <array>
#include <cmath>
constexpr float pi = 3.14159265358979323846f;
struct Vec3 {
    float x, y, z;
};

inline Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vec3 operator*(Vec3 a, float k) { return {a.x * k, a.y * k, a.z * k}; }
inline float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
inline Vec3 normalized(Vec3 v) {
    const float length = std::sqrt(dot(v, v));
    return v * (1.0f / length);
}

struct Mat4 {
    std::array<float, 16> values{}; // OpenGL usa ordem por colunas.
};

inline Mat4 identity() {
    Mat4 m;
    m.values[0] = m.values[5] = m.values[10] = m.values[15] = 1;
    return m;
}
inline Mat4 operator*(const Mat4& a, const Mat4& b) {
    Mat4 m;
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            for (int k = 0; k < 4; ++k)
                m.values[c*4+r] += a.values[k*4+r] * b.values[c*4+k];
    return m;
}
inline Mat4 translate(Vec3 p) {
    Mat4 m = identity();
    m.values[12] = p.x; m.values[13] = p.y; m.values[14] = p.z;
    return m;
}
inline Mat4 scale(Vec3 s) {
    Mat4 m = identity();
    m.values[0] = s.x; m.values[5] = s.y; m.values[10] = s.z;
    return m;
}
inline Mat4 rotateY(float angle) {
    Mat4 m = identity();
    m.values[0] = m.values[10] = std::cos(angle);
    m.values[8] = std::sin(angle); m.values[2] = -std::sin(angle);
    return m;
}
inline Mat4 rotateX(float angle) {
    Mat4 m = identity();
    m.values[5] = m.values[10] = std::cos(angle);
    m.values[6] = std::sin(angle); m.values[9] = -std::sin(angle);
    return m;
}
inline Mat4 rotateZ(float angle) {
    Mat4 m = identity();
    m.values[0] = m.values[5] = std::cos(angle);
    m.values[1] = std::sin(angle); m.values[4] = -std::sin(angle);
    return m;
}
inline Mat4 model(Vec3 position, Vec3 size, float yaw = 0) {
    return translate(position) * rotateY(yaw) * scale(size);
}
inline Vec3 transformPoint(const Mat4& m, Vec3 p) {
    const auto& a = m.values;
    return {a[0]*p.x+a[4]*p.y+a[8]*p.z+a[12],
            a[1]*p.x+a[5]*p.y+a[9]*p.z+a[13],
            a[2]*p.x+a[6]*p.y+a[10]*p.z+a[14]};
}

inline Mat4 perspective(float fovRadians, float aspect, float nearPlane, float farPlane) {
    const float f = 1.0f / std::tan(fovRadians * 0.5f);
    Mat4 m;
    m.values[0] = f / aspect;
    m.values[5] = f;
    m.values[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
    m.values[11] = -1.0f;
    m.values[14] = 2.0f * farPlane * nearPlane / (nearPlane - farPlane);
    return m;
}

inline Mat4 lookAt(Vec3 eye, Vec3 target) {
    const Vec3 forward = normalized(target - eye);
    // Uma fonte vertical (a luminaria) precisa de um eixo auxiliar diferente.
    const Vec3 reference = std::abs(forward.y) > .99f ? Vec3{0,0,-1} : Vec3{0,1,0};
    const Vec3 right = normalized(cross(forward, reference));
    const Vec3 up = cross(right, forward);
    Mat4 m;
    m.values = {right.x, up.x, -forward.x, 0.0f,
                right.y, up.y, -forward.y, 0.0f,
                right.z, up.z, -forward.z, 0.0f,
                -dot(right, eye), -dot(up, eye), dot(forward, eye), 1.0f};
    return m;
}

