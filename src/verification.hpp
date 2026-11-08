#pragma once
#include "behavior.hpp"

struct VerificationFrame {
    Scene cena;
    Vec3 camera, alvo;
    std::string arquivo;
};
std::vector<VerificationFrame> makeVerificationFrames();
