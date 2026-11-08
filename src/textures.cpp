#include "scene.hpp"
#include <algorithm>

std::vector<std::uint8_t> makeTexture(TextureKind tipo, int size) {
    std::vector<std::uint8_t> pixels(size*size*3);
    for (int y=0; y<size; ++y) for (int x=0; x<size; ++x) {
        const float u=(x+.5f)/size, v=(y+.5f)/size;
        // Ruido deterministico: a mesma textura em cada execucao.
        const std::uint32_t hash=(std::uint32_t(x)*73856093u)^(std::uint32_t(y)*19349663u);
        const float ruido=float(hash%1000)/1000.f;
        Vec3 cor{1,1,1};
        if (tipo==TextureKind::Wood || tipo==TextureKind::Floor) {
            const float veio=.92f+.055f*std::sin(2*pi*(v*42+.18f*std::sin(u*2*pi)))+.035f*ruido;
            cor=Vec3{.67f,.43f,.23f}*veio;
            if (tipo==TextureKind::Floor) {
                const int linha=int(v*6);
                const float aoLongo=std::fmod(u+(linha%2)*.5f,1.f);
                cor=Vec3{.66f,.48f,.31f}*(veio*(.94f+.018f*linha));
                if (std::fmod(v*6,1.f)<.025f || aoLongo<.007f) cor=cor*.48f;
            }
        } else if (tipo==TextureKind::Ball) {
            Vec3 n{std::sin(v*pi)*std::cos(u*2*pi),std::cos(v*pi),std::sin(v*pi)*std::sin(u*2*pi)};
            const bool costura=std::abs(n.x)<.026f || std::abs(n.y)<.026f || std::abs(n.z)<.026f;
            cor=costura ? Vec3{.055f,.037f,.025f} : Vec3{.93f,.35f,.055f}*(.84f+.16f*ruido);
        } else if (tipo==TextureKind::Screen) {
            cor={.05f+.05f*v,.13f+.12f*v,.21f+.15f*v};
            if (u>.08f && u<.90f && v>.15f && v<.85f) cor={.10f,.21f,.29f};
            if (u>.12f && u<.60f && v>.25f && v<.78f && int(v*24)%3==0) cor={.38f,.74f,.69f};
            if (v<.07f) cor={.20f,.33f,.39f};
        }
        const int i=(y*size+x)*3;
        pixels[i]=static_cast<std::uint8_t>(std::clamp(cor.x,0.f,1.f)*255);
        pixels[i+1]=static_cast<std::uint8_t>(std::clamp(cor.y,0.f,1.f)*255);
        pixels[i+2]=static_cast<std::uint8_t>(std::clamp(cor.z,0.f,1.f)*255);
    }
    return pixels;
}
