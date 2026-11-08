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
    std::array<float, 16> valores{}; // OpenGL usa ordem por colunas.
};

inline Mat4 identity() {
    Mat4 m;
    m.valores[0] = m.valores[5] = m.valores[10] = m.valores[15] = 1;
    return m;
}
inline Mat4 operator*(const Mat4& a, const Mat4& b) {
    Mat4 m;
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            for (int k = 0; k < 4; ++k)
                m.valores[c*4+r] += a.valores[k*4+r] * b.valores[c*4+k];
    return m;
}
inline Mat4 translate(Vec3 p) {
    Mat4 m = identity();
    m.valores[12] = p.x; m.valores[13] = p.y; m.valores[14] = p.z;
    return m;
}
inline Mat4 scale(Vec3 s) {
    Mat4 m = identity();
    m.valores[0] = s.x; m.valores[5] = s.y; m.valores[10] = s.z;
    return m;
}
inline Mat4 rotateY(float angulo) {
    Mat4 m = identity();
    m.valores[0] = m.valores[10] = std::cos(angulo);
    m.valores[8] = std::sin(angulo); m.valores[2] = -std::sin(angulo);
    return m;
}
inline Mat4 rotateX(float angulo) {
    Mat4 m = identity();
    m.valores[5] = m.valores[10] = std::cos(angulo);
    m.valores[6] = std::sin(angulo); m.valores[9] = -std::sin(angulo);
    return m;
}
inline Mat4 rotateZ(float angulo) {
    Mat4 m = identity();
    m.valores[0] = m.valores[5] = std::cos(angulo);
    m.valores[1] = std::sin(angulo); m.valores[4] = -std::sin(angulo);
    return m;
}
inline Mat4 model(Vec3 posicao, Vec3 size, float guinada = 0) {
    return translate(posicao) * rotateY(guinada) * scale(size);
}
inline Vec3 transformPoint(const Mat4& m, Vec3 p) {
    const auto& a = m.valores;
    return {a[0]*p.x+a[4]*p.y+a[8]*p.z+a[12],
            a[1]*p.x+a[5]*p.y+a[9]*p.z+a[13],
            a[2]*p.x+a[6]*p.y+a[10]*p.z+a[14]};
}

inline Mat4 perspective(float campoVisaoRadianos, float proporcao, float planoProximo, float planoDistante) {
    const float f = 1.0f / std::tan(campoVisaoRadianos * 0.5f);
    Mat4 m;
    m.valores[0] = f / proporcao;
    m.valores[5] = f;
    m.valores[10] = (planoDistante + planoProximo) / (planoProximo - planoDistante);
    m.valores[11] = -1.0f;
    m.valores[14] = 2.0f * planoDistante * planoProximo / (planoProximo - planoDistante);
    return m;
}

inline Mat4 lookAt(Vec3 olho, Vec3 alvo) {
    const Vec3 frente = normalized(alvo - olho);
    // Uma fonte vertical (a luminaria) precisa de um eixo auxiliar diferente.
    const Vec3 referencia = std::abs(frente.y) > .99f ? Vec3{0,0,-1} : Vec3{0,1,0};
    const Vec3 direita = normalized(cross(frente, referencia));
    const Vec3 cima = cross(direita, frente);
    Mat4 m;
    m.valores = {direita.x, cima.x, -frente.x, 0.0f,
                direita.y, cima.y, -frente.y, 0.0f,
                direita.z, cima.z, -frente.z, 0.0f,
                -dot(direita, olho), -dot(cima, olho), dot(frente, olho), 1.0f};
    return m;
}

