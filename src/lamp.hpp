#pragma once
#include "math.hpp"

// Geometria e shader compartilham a mesma referencia da luminaria.
constexpr Vec3 lampAnchor{4.25f,0.f,2.30f};
constexpr float lampShadeY=2.35f, lampShadeHeight=.24f;
constexpr float lampBulbY=2.20f, lampBulbHeight=.16f;
constexpr Vec3 lampEmitter{lampAnchor.x,lampBulbY-lampBulbHeight*.5f-.005f,lampAnchor.z};
constexpr Vec3 lampDirection{0.f,-1.f,0.f};
// Angulos aproximados: centro pleno ate 41 graus; borda suave ate 61 graus.
constexpr float lampConeInnerCos=.75f, lampConeOuterCos=.48f;
