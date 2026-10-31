#pragma once
#include "behavior.hpp"

struct VerificationFrame {
    Scene scene;
    Vec3 camera, target;
    std::string file;
};
std::vector<VerificationFrame> makeVerificationFrames();
