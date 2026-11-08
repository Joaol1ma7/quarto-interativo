#pragma once
#include "math.hpp"
#include <cstdint>
#include <string>
#include <vector>

constexpr float larguraQuarto = 8.5f, profundidadeQuarto = 4.75f, alturaQuarto = 2.8f;
constexpr float larguraRecuo = 0.9f, inicioRecuo = 3.28f;
constexpr float janelaZMin=1.85f, janelaZMax=3.12f;
constexpr float baseJanela=.85f, topoJanela=2.10f;
constexpr float portaZMin=3.60f, portaZMax=4.55f, topoPorta=2.20f;
constexpr float raioCamera = 0.18f;
constexpr Vec3 cameraInicial{7.15f,1.65f,3.70f};
constexpr Vec3 alvoInicial{3.0f,.95f,1.6f};

struct Vec2 { float x, y; };
struct Vertex { Vec3 posicao, normal, cor; Vec2 uv; };
using MeshData = std::vector<Vertex>;
enum class MeshKind { Cube, Sphere, Cylinder, Shade, Shell, Floor, Count };
enum class TextureKind { White, Floor, Wood, Ball, Art, Screen, Count };
enum class MaterialId {
    Wall, Floor, Wood, DarkWood, Fabric, ChairFabric, WhiteFabric, Metal, Plastic,
    Glass, Screen, Art, Ball, Light, KeyPlastic,
    Grass, HillFar, HillNear, Leaves, Bark, Path, Cloud, Sun, Moon, Star, Count
};
struct Material {
    Vec3 cor;
    float especular, brilho, alpha, emissao;
    TextureKind texture;
};

// Cada peca conserva sua matriz local. As partes moveis compartilham um pivo.
enum class Group { Static, Door, Window, Ball, Sun };
struct Part {
    std::string objeto;
    MeshKind malha;
    MaterialId material;
    Mat4 local;
    Group grupo = Group::Static;
    bool externo = false;
    unsigned char mascaraPeriodo = 7; // bit 0: manha; bit 1: tarde; bit 2: noite
};
struct Collider { Vec2 centro, meiaExtensao; float guinada = 0; };
enum class DayPeriod { Morning, Afternoon, Night };
struct Scene {
    std::vector<Part> partes;
    std::vector<Collider> colisores;
    float anguloPorta = 0, anguloJanela = 0;
    Vec3 posicaoBola{2.4f, 0.20f, 2.28f};
    float rotacaoBola = 0;
    Mat4 orientacaoBola = identity();
    Vec3 velocidadeBola{0,0,0};
    bool bolaProxima = false;
    bool portaAberta = false, janelaAberta = false, luminariaLigada = true;
    bool portaBloqueada = false, janelaBloqueada = false;
    DayPeriod periodo = DayPeriod::Morning;
};

MeshData makeMesh(MeshKind tipo);
std::vector<Material> makeMaterials();
std::vector<std::uint8_t> makeTexture(TextureKind tipo, int size);
Scene makeScene();
void addLandscape(Scene& cena);
Mat4 partModel(const Scene& cena, const Part& parte);
inline bool visiblePart(const Scene& cena, const Part& parte) {
    return (parte.mascaraPeriodo & (1u << static_cast<int>(cena.periodo))) != 0;
}
bool canWalk(const Scene& cena, float x, float z);
