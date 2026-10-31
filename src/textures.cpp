#include "scene.hpp"
#include <algorithm>

std::vector<std::uint8_t> makeTexture(TextureKind kind, int size) {
    std::vector<std::uint8_t> pixels(size*size*3);
    for (int y=0; y<size; ++y) for (int x=0; x<size; ++x) {
        const float u=(x+.5f)/size, v=(y+.5f)/size;
        // Ruido deterministico: a mesma textura em cada execucao.
        const std::uint32_t hash=(std::uint32_t(x)*73856093u)^(std::uint32_t(y)*19349663u);
        const float noise=float(hash%1000)/1000.f;
        Vec3 color{1,1,1};
        if (kind==TextureKind::Wood || kind==TextureKind::Floor) {
            const float grain=.92f+.055f*std::sin(2*pi*(v*42+.18f*std::sin(u*2*pi)))+.035f*noise;
            color=Vec3{.67f,.43f,.23f}*grain;
            if (kind==TextureKind::Floor) {
                const int row=int(v*6);
                const float along=std::fmod(u+(row%2)*.5f,1.f);
                color=Vec3{.66f,.48f,.31f}*(grain*(.94f+.018f*row));
                if (std::fmod(v*6,1.f)<.025f || along<.007f) color=color*.48f;
            }
        } else if (kind==TextureKind::Ball) {
            Vec3 n{std::sin(v*pi)*std::cos(u*2*pi),std::cos(v*pi),std::sin(v*pi)*std::sin(u*2*pi)};
            const bool seam=std::abs(n.x)<.026f || std::abs(n.y)<.026f || std::abs(n.z)<.026f;
            color=seam ? Vec3{.055f,.037f,.025f} : Vec3{.93f,.35f,.055f}*(.84f+.16f*noise);
        } else if (kind==TextureKind::Screen) {
            color={.05f+.05f*v,.13f+.12f*v,.21f+.15f*v};
            if (u>.08f && u<.90f && v>.15f && v<.85f) color={.10f,.21f,.29f};
            if (u>.12f && u<.60f && v>.25f && v<.78f && int(v*24)%3==0) color={.38f,.74f,.69f};
            if (v<.07f) color={.20f,.33f,.39f};
        }
        const int i=(y*size+x)*3;
        pixels[i]=static_cast<std::uint8_t>(std::clamp(color.x,0.f,1.f)*255);
        pixels[i+1]=static_cast<std::uint8_t>(std::clamp(color.y,0.f,1.f)*255);
        pixels[i+2]=static_cast<std::uint8_t>(std::clamp(color.z,0.f,1.f)*255);
    }
    return pixels;
}
