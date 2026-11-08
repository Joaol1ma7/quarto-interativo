#pragma once
#include <cstdint>
#include <vector>

struct ImageData {
    int largura = 0;
    int altura = 0;
    std::vector<std::uint8_t> pixels;
};

// Carrega a fotografia copiada ao lado do executavel em assets/quadro.jpg.
ImageData loadArtworkImage();
