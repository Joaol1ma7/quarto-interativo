#pragma once
#include <cstdint>
#include <vector>

struct ImageData {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> pixels;
};

// Carrega a fotografia copiada ao lado do executavel em assets/quadro.jpg.
ImageData loadArtworkImage();
