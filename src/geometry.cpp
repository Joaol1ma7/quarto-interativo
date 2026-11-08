#include "scene.hpp"

namespace {
void addQuad(std::vector<Vertex>& malha, Vec3 a, Vec3 b, Vec3 c, Vec3 d,
             Vec3 normal, Vec3 cor) {
    const Vec2 uv[] = {{0,0},{1,0},{1,1},{0,0},{1,1},{0,1}};
    int i = 0;
    for (Vec3 p : {a, b, c, a, c, d}) malha.push_back({p, normal, cor, uv[i++]});
}

void addWall(std::vector<Vertex>& malha, float x1, float z1, float x2, float z2,
             float baixo, float alto, Vec3 normal, Vec3 cor) {
    addQuad(malha, {x1, baixo, z1}, {x2, baixo, z2}, {x2, alto, z2}, {x1, alto, z1},
            normal, cor);
}

std::vector<Vertex> makeRoom() {
    std::vector<Vertex> malha;
    const Vec3 corTeto{0.85f, 0.87f, 0.86f};
    const Vec3 corParede{0.78f, 0.82f, 0.79f};
    const Vec3 corLateral{0.69f, 0.76f, 0.78f};
    const Vec3 corMoldura{0.34f, 0.39f, 0.39f};

    // A planta e um L: o canto inferior esquerdo recua 0,9 m.
    auto horizontal = [&](float y, Vec3 normal, Vec3 cor) {
        addQuad(malha, {0, y, 0}, {larguraQuarto, y, 0},
                {larguraQuarto, y, inicioRecuo}, {0, y, inicioRecuo}, normal, cor);
        addQuad(malha, {larguraRecuo, y, inicioRecuo},
                {larguraQuarto, y, inicioRecuo}, {larguraQuarto, y, profundidadeQuarto},
                {larguraRecuo, y, profundidadeQuarto}, normal, cor);
    };
    // O piso usa outra malha, para receber sua propria textura.
    horizontal(alturaQuarto, {0, -1, 0}, corTeto);

    addWall(malha, 0, 0, larguraQuarto, 0, 0, alturaQuarto, {0, 0, 1}, corParede);
    addWall(malha, larguraRecuo, profundidadeQuarto, larguraQuarto, profundidadeQuarto,
            0, alturaQuarto, {0, 0, -1}, corParede);
    addWall(malha, 0, inicioRecuo, larguraRecuo, inicioRecuo,
            0, alturaQuarto, {0, 0, -1}, corLateral);
    addWall(malha, larguraRecuo, inicioRecuo, larguraRecuo, profundidadeQuarto,
            0, alturaQuarto, {1, 0, 0}, corLateral);

    // Abertura real da janela; os mesmos limites orientam a luz natural.
    addWall(malha, 0, 0, 0, janelaZMin, 0, alturaQuarto, {1, 0, 0}, corLateral);
    addWall(malha, 0, janelaZMin, 0, janelaZMax, 0, baseJanela, {1, 0, 0}, corLateral);
    addWall(malha, 0, janelaZMin, 0, janelaZMax, topoJanela, alturaQuarto, {1, 0, 0}, corLateral);
    addWall(malha, 0, janelaZMax, 0, inicioRecuo, 0, alturaQuarto, {1, 0, 0}, corLateral);

    // Parede direita: vao de porta do piso ate 2,20 m.
    addWall(malha, larguraQuarto, 0, larguraQuarto, portaZMin,
            0, alturaQuarto, {-1, 0, 0}, corParede);
    addWall(malha, larguraQuarto, portaZMin, larguraQuarto, portaZMax,
            topoPorta, alturaQuarto, {-1, 0, 0}, corParede);
    addWall(malha, larguraQuarto, portaZMax, larguraQuarto, profundidadeQuarto,
            0, alturaQuarto, {-1, 0, 0}, corParede);

    // Molduras planas ajudam a reconhecer os dois vaos nesta fase sem esquadrias.
    constexpr float molduraEsquerdaX = 0.008f;
    addWall(malha, molduraEsquerdaX, 1.82f, molduraEsquerdaX, 1.86f,
            0.82f, 2.13f, {1, 0, 0}, corMoldura);
    addWall(malha, molduraEsquerdaX, 3.11f, molduraEsquerdaX, 3.15f,
            0.82f, 2.13f, {1, 0, 0}, corMoldura);
    addWall(malha, molduraEsquerdaX, 1.85f, molduraEsquerdaX, 3.12f,
            0.82f, 0.86f, {1, 0, 0}, corMoldura);
    addWall(malha, molduraEsquerdaX, 1.85f, molduraEsquerdaX, 3.12f,
            2.09f, 2.13f, {1, 0, 0}, corMoldura);

    constexpr float molduraDireitaX = larguraQuarto - 0.008f;
    addWall(malha, molduraDireitaX, 3.57f, molduraDireitaX, 3.61f,
            0, 2.23f, {-1, 0, 0}, corMoldura);
    addWall(malha, molduraDireitaX, 4.54f, molduraDireitaX, 4.58f,
            0, 2.23f, {-1, 0, 0}, corMoldura);
    addWall(malha, molduraDireitaX, 3.60f, molduraDireitaX, 4.55f,
            2.19f, 2.23f, {-1, 0, 0}, corMoldura);
    return malha;
}


} // namespace

MeshData makeMesh(MeshKind tipo) {
    if (tipo == MeshKind::Shell) return makeRoom();
    MeshData m;
    const Vec3 branco{1,1,1};
    if (tipo == MeshKind::Floor) {
        addQuad(m, {0,0,0}, {larguraQuarto,0,0}, {larguraQuarto,0,inicioRecuo},
                {0,0,inicioRecuo}, {0,1,0}, branco);
        addQuad(m, {larguraRecuo,0,inicioRecuo}, {larguraQuarto,0,inicioRecuo},
                {larguraQuarto,0,profundidadeQuarto}, {larguraRecuo,0,profundidadeQuarto}, {0,1,0}, branco);
        for (auto& v : m) v.uv = {v.posicao.x / 1.8f, v.posicao.z / 1.8f};
    } else if (tipo == MeshKind::Cube) {
        addQuad(m, {-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f},{0,0,1},branco);
        addQuad(m, {.5f,-.5f,-.5f},{-.5f,-.5f,-.5f},{-.5f,.5f,-.5f},{.5f,.5f,-.5f},{0,0,-1},branco);
        addQuad(m, {-.5f,-.5f,-.5f},{-.5f,-.5f,.5f},{-.5f,.5f,.5f},{-.5f,.5f,-.5f},{-1,0,0},branco);
        addQuad(m, {.5f,-.5f,.5f},{.5f,-.5f,-.5f},{.5f,.5f,-.5f},{.5f,.5f,.5f},{1,0,0},branco);
        addQuad(m, {-.5f,.5f,.5f},{.5f,.5f,.5f},{.5f,.5f,-.5f},{-.5f,.5f,-.5f},{0,1,0},branco);
        addQuad(m, {-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,-.5f,.5f},{-.5f,-.5f,.5f},{0,-1,0},branco);
    } else if (tipo == MeshKind::Sphere) {
        constexpr int fatias = 48, camadas = 24;
        auto vertice = [&](int x, int y) {
            const float u = float(x)/fatias, v = float(y)/camadas;
            const float teta = u*2*pi, fi = v*pi;
            Vec3 n{std::sin(fi)*std::cos(teta), std::cos(fi), std::sin(fi)*std::sin(teta)};
            return Vertex{n*.5f, n, branco, {u,v}};
        };
        for (int y=0; y<camadas; ++y) for (int x=0; x<fatias; ++x) {
            const Vertex a=vertice(x,y), b=vertice(x+1,y), c=vertice(x+1,y+1), d=vertice(x,y+1);
            if (y>0) m.insert(m.end(), {a,c,b});
            if (y<camadas-1) m.insert(m.end(), {a,d,c});
        }
    } else {
        // Cilindro e cupula: mesma construcao, com raios diferentes no topo.
        const float raioTopo = tipo == MeshKind::Shade ? .20f : .5f;
        constexpr int fatias=32;
        for (int i=0; i<fatias; ++i) {
            const float a=2*pi*i/fatias, b=2*pi*(i+1)/fatias;
            Vec3 na=normalized({std::cos(a), .5f-raioTopo, std::sin(a)});
            Vec3 nb=normalized({std::cos(b), .5f-raioTopo, std::sin(b)});
            Vertex p{{.5f*std::cos(a),-.5f,.5f*std::sin(a)},na,branco,{float(i)/fatias,0}};
            Vertex q{{.5f*std::cos(b),-.5f,.5f*std::sin(b)},nb,branco,{float(i+1)/fatias,0}};
            Vertex r{{raioTopo*std::cos(b),.5f,raioTopo*std::sin(b)},nb,branco,{float(i+1)/fatias,1}};
            Vertex s{{raioTopo*std::cos(a),.5f,raioTopo*std::sin(a)},na,branco,{float(i)/fatias,1}};
            m.insert(m.end(), {p,r,q,p,s,r});
            auto tampa = [&](Vertex v, float sign) {
                v.normal={0,sign,0}; v.uv={v.posicao.x+.5f,v.posicao.z+.5f}; return v;
            };
            // A cupula conserva laterais e topo opacos, mas tem abertura inferior.
            if (tipo != MeshKind::Shade)
                m.insert(m.end(), {{{0,-.5f,0},{0,-1,0},branco,{.5f,.5f}},tampa(p,-1),tampa(q,-1)});
            m.insert(m.end(), {{{0,.5f,0},{0,1,0},branco,{.5f,.5f}},tampa(r,1),tampa(s,1)});
        }
    }
    return m;
}
