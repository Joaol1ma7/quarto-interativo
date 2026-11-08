#include "scene.hpp"
#include "lamp.hpp"
#include "celestial.hpp"
#include <algorithm>

namespace {
// Uma composicao e somente uma lista de primitivas com transformacoes locais.
struct ObjectBuilder {
    Scene& cena;
    const char* nome;
    Mat4 base = identity();
    Group grupo = Group::Static;
    void parte(MeshKind malha, MaterialId material, Vec3 p, Vec3 size, float guinada=0) {
        cena.partes.push_back({nome,malha,material,base*model(p,size,guinada),grupo});
    }
    void box(MaterialId material, Vec3 p, Vec3 size, float guinada=0) {
        parte(MeshKind::Cube,material,p,size,guinada);
    }
};
using M = MaterialId;

void addBed(Scene& cena) {
    ObjectBuilder b{cena,"cama",translate({6.82f,0,.88f})};
    b.box(M::DarkWood,{0,.27f,0},{2.72f,.20f,1.46f});
    for (float x : {-1.16f,1.16f}) for (float z : {-.57f,.57f})
        b.box(M::Wood,{x,.13f,z},{.14f,.26f,.14f});
    b.box(M::WhiteFabric,{0,.48f,0},{2.56f,.24f,1.34f});
    b.box(M::Fabric,{-.34f,.625f,0},{1.86f,.055f,1.36f});
    // Laterais da colcha caem sobre o colchao, separadas da base de madeira.
    for (float z : {-.68f,.68f}) b.box(M::Fabric,{-.34f,.49f,z},{1.86f,.27f,.025f});
    b.parte(MeshKind::Sphere,M::WhiteFabric,{.97f,.67f,0},{.46f,.19f,1.02f},-.06f);
    b.box(M::Wood,{1.37f,.69f,0},{.11f,1.15f,1.5f});
    b.box(M::Wood,{-1.37f,.39f,0},{.09f,.48f,1.5f});
    cena.colisores.push_back({{6.82f,.88f},{1.43f,.76f}});
}

void addWardrobe(Scene& cena) {
    ObjectBuilder b{cena,"guarda-roupas",translate({2.82f,0,4.36f})};
    b.box(M::DarkWood,{0,1.10f,.31f},{3.46f,2.20f,.07f});
    for (float x : {-1.70f,1.70f}) b.box(M::Wood,{x,1.10f,0},{.07f,2.20f,.68f});
    for (float y : {.06f,2.20f}) b.box(M::Wood,{0,y,0},{3.48f,.12f,.70f});
    b.box(M::DarkWood,{0,.10f,-.20f},{3.30f,.16f,.26f});
    for (float x : {-1.13f,0.f,1.13f}) {
        b.box(M::Wood,{x,1.15f,-.33f},{1.08f,2.02f,.065f});
        b.box(M::DarkWood,{x,1.17f,-.366f},{.92f,1.78f,.018f});
        b.box(M::Wood,{x,1.17f,-.38f},{.86f,1.72f,.018f});
        b.box(M::Metal,{x+.35f,1.15f,-.42f},{.035f,.25f,.04f});
    }
    cena.colisores.push_back({{2.82f,4.33f},{1.76f,.41f}});
}

void addDesk(Scene& cena) {
    ObjectBuilder b{cena,"mesa",translate({1.23f,0,.58f})};
    b.box(M::Wood,{0,.78f,0},{1.96f,.09f,.84f});
    for (float x : {-.84f,.84f}) for (float z : {-.30f,.30f})
        b.box(M::Metal,{x,.37f,z},{.055f,.74f,.055f});
    b.box(M::DarkWood,{-.66f,.52f,0},{.45f,.44f,.65f});
    for (float y : {.43f,.62f}) {
        b.box(M::Wood,{-.66f,y,.34f},{.43f,.17f,.035f});
        b.box(M::Metal,{-.66f,y,.37f},{.14f,.025f,.025f});
    }
    cena.colisores.push_back({{1.23f,.58f},{.99f,.43f}});
    // Gabinete do computador, alem dos onze objetos pedidos.
    ObjectBuilder pc{cena,"computador",translate({1.99f,.83f,.43f})};
    pc.box(M::Plastic,{0,.24f,0},{.25f,.48f,.33f});
    pc.box(M::Metal,{0,.27f,.172f},{.19f,.31f,.012f});
    pc.parte(MeshKind::Sphere,M::Light,{0,.41f,.187f},{.022f,.022f,.012f});
    for (int i=0;i<5;++i) pc.box(M::Plastic,{0,.18f+i*.035f,.184f},{.15f,.012f,.008f});
}

void addChair(Scene& cena) {
    constexpr float guinada = -.14f;
    ObjectBuilder b{cena,"cadeira",translate({1.28f,0,1.48f})*rotateY(guinada)};
    b.box(M::DarkWood,{0,.46f,0},{.58f,.08f,.55f});
    b.box(M::ChairFabric,{0,.515f,0},{.54f,.07f,.52f});
    for (float x : {-.23f,.23f}) for (float z : {-.21f,.21f})
        b.box(M::Metal,{x,.22f,z},{.045f,.44f,.045f});
    for (float x : {-.23f,.23f}) b.box(M::Metal,{x,.76f,.24f},{.04f,.58f,.04f});
    b.box(M::DarkWood,{0,.88f,.25f},{.57f,.34f,.07f});
    b.box(M::ChairFabric,{0,.88f,.204f},{.49f,.27f,.03f});
    cena.colisores.push_back({{1.28f,1.48f},{.30f,.30f},guinada});
}

void addMonitor(Scene& cena) {
    ObjectBuilder b{cena,"monitor",translate({1.18f,.83f,.37f})};
    b.box(M::Plastic,{0,.022f,.03f},{.34f,.044f,.23f});
    b.box(M::Metal,{0,.14f,0},{.07f,.24f,.06f});
    b.box(M::Plastic,{0,.37f,0},{.85f,.50f,.065f});
    b.box(M::Screen,{0,.38f,.038f},{.77f,.42f,.012f});
    b.parte(MeshKind::Sphere,M::Light,{.34f,.137f,.038f},{.012f,.012f,.008f});
}

void addKeyboard(Scene& cena) {
    ObjectBuilder b{cena,"teclado",translate({1.18f,.845f,.82f})*rotateY(-.06f)};
    b.box(M::Plastic,{0,0,0},{.78f,.04f,.24f});
    for (int linha=0;linha<4;++linha) for (int coluna=0;coluna<12;++coluna) {
        if (linha==3 && coluna>=3 && coluna<=8) continue;
        b.box(M::KeyPlastic,{-.34f+coluna*.062f,.027f,-.082f+linha*.054f},{.048f,.018f,.039f});
    }
    b.box(M::KeyPlastic,{0,.027f,.080f},{.35f,.018f,.039f});
}

void addDoor(Scene& cena) {
    ObjectBuilder b{cena,"porta",identity(),Group::Door};
    b.box(M::Wood,{0,1.09f,.475f},{.06f,2.18f,.94f});
    for (float y : {.57f,1.58f}) {
        b.box(M::DarkWood,{-.034f,y,.475f},{.014f,.78f,.72f});
        b.box(M::Wood,{-.044f,y,.475f},{.014f,.71f,.65f});
    }
    b.box(M::Metal,{-.07f,1.06f,.80f},{.06f,.12f,.05f});
    b.box(M::Metal,{-.11f,1.06f,.74f},{.035f,.035f,.16f});
    for (float y : {.30f,1.85f})
        b.parte(MeshKind::Cylinder,M::Metal,{0,y,.015f},{.045f,.13f,.045f});
}

void addWindow(Scene& cena) {
    ObjectBuilder b{cena,"janela",identity(),Group::Window};
    for (float y : {.025f,1.225f}) b.box(M::Metal,{0,y,.635f},{.08f,.05f,1.27f});
    for (float z : {.025f,.635f,1.245f}) b.box(M::Metal,{0,.625f,z},{.08f,1.20f,.05f});
    for (float z : {.33f,.94f}) b.box(M::Glass,{0,.625f,z},{.015f,1.15f,.56f});
    b.box(M::Plastic,{.075f,.58f,1.17f},{.06f,.17f,.025f});
    ObjectBuilder sill{cena,"janela"};
    sill.box(M::Wood,{.07f,.83f,2.485f},{.22f,.06f,1.40f});
}

void addLamp(Scene& cena) {
    ObjectBuilder b{cena,"luminaria",translate(lampAnchor)};
    b.parte(MeshKind::Cylinder,M::Metal,{0,2.76f,0},{.20f,.08f,.20f});
    b.parte(MeshKind::Cylinder,M::Metal,{0,2.58f,0},{.025f,.30f,.025f});
    b.parte(MeshKind::Shade,M::WhiteFabric,{0,lampShadeY,0},{.62f,lampShadeHeight,.62f});
    b.parte(MeshKind::Sphere,M::Light,{0,lampBulbY,0},{.15f,lampBulbHeight,.15f});
    ObjectBuilder sw{cena,"interruptor"};
    sw.box(M::KeyPlastic,{8.455f,1.30f,3.25f},{.045f,.20f,.13f});
    sw.box(M::Plastic,{8.425f,1.30f,3.25f},{.025f,.11f,.055f});
}

void addPicture(Scene& cena) {
    ObjectBuilder b{cena,"quadro",translate({3.75f,1.67f,.035f})};
    // Proporcao 1,50 da fotografia original, sem esticar ou cortar a imagem.
    b.box(M::Art,{0,0,.025f},{1.54f,1.025f,.035f});
    for (float y : {-.55f,.55f}) b.box(M::DarkWood,{0,y,.04f},{1.68f,.07f,.08f});
    for (float x : {-.81f,.81f}) b.box(M::DarkWood,{x,0,.04f},{.07f,1.17f,.08f});
}
} // namespace

std::vector<Material> makeMaterials() {
    // Cor, intensidade especular, expoente do brilho, opacidade, emissao, textura.
    return {
        {{1,1,1},.012f,12,1,0,TextureKind::White},
        {{1,1,1},.10f,24,1,0,TextureKind::Floor},
        {{1,.88f,.72f},.18f,18,1,0,TextureKind::Wood},
        {{.48f,.37f,.27f},.14f,18,1,0,TextureKind::Wood},
        {{.09f,.43f,.15f},.008f,9,1,0,TextureKind::White}, // colcha verde
        {{.055f,.060f,.065f},.008f,9,1,0,TextureKind::White}, // cadeira preta
        {{.93f,.91f,.82f},.008f,9,1,0,TextureKind::White},
        {{.58f,.64f,.67f},1.30f,80,1,0,TextureKind::White},
        {{.045f,.058f,.074f},.48f,32,1,0,TextureKind::White},
        {{.82f,.92f,1.f},.90f,80,.14f,0,TextureKind::White},
        {{1,1,1},.18f,60,1,.45f,TextureKind::Screen},
        {{1,1,1},.03f,6,1,0,TextureKind::Art},
        {{1,1,1},.07f,12,1,0,TextureKind::Ball},
        {{1,.89f,.61f},.10f,16,1,.85f,TextureKind::White},
        {{.65f,.68f,.66f},.35f,28,1,0,TextureKind::White},
        {{.33f,.62f,.25f},.03f,8,1,0,TextureKind::White},   // grama
        {{.39f,.55f,.52f},.02f,8,1,0,TextureKind::White},   // colinas distantes
        {{.30f,.51f,.33f},.02f,8,1,0,TextureKind::White},   // colinas proximas
        {{.24f,.51f,.22f},.04f,12,1,0,TextureKind::White},  // copa
        {{.38f,.23f,.12f},.02f,8,1,0,TextureKind::White},   // tronco
        {{.70f,.55f,.35f},.03f,8,1,0,TextureKind::White},   // caminho
        {{.95f,.95f,.91f},.02f,8,1,0,TextureKind::White},   // nuvens
        {{1.f,.90f,.53f},0,1,1,1,TextureKind::White},        // sol
        {{.89f,.93f,1.f},0,1,1,1,TextureKind::White},        // lua
        {{.91f,.94f,1.f},0,1,1,1,TextureKind::White}         // estrelas
    };
}

Scene makeScene() {
    Scene s;
    s.partes.push_back({"quarto",MeshKind::Shell,M::Wall,identity()});
    s.partes.push_back({"piso",MeshKind::Floor,M::Floor,identity()});
    addBed(s); addWardrobe(s); addDesk(s); addChair(s); addMonitor(s);
    addKeyboard(s); addDoor(s); addWindow(s); addLamp(s); addPicture(s);
    ObjectBuilder ball{s,"bola",identity(),Group::Ball};
    ball.parte(MeshKind::Sphere,M::Ball,{0,0,0},{.40f,.40f,.40f});
    addLandscape(s);
    return s;
}

Mat4 partModel(const Scene& cena, const Part& parte) {
    switch (parte.grupo) {
    case Group::Door: return translate({8.46f,0,3.60f})*rotateY(cena.anguloPorta)*parte.local;
    case Group::Window: return translate({.04f,.85f,1.85f})*rotateY(cena.anguloJanela)*parte.local;
    case Group::Ball: return translate(cena.posicaoBola)*cena.orientacaoBola*parte.local;
    case Group::Sun: return translate(celestialVisualPosition(cena.periodo))*parte.local;
    default: return parte.local;
    }
}

