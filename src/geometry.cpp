#include "scene.hpp"

namespace {
void addQuad(std::vector<Vertex>& mesh, Vec3 a, Vec3 b, Vec3 c, Vec3 d,
             Vec3 normal, Vec3 color) {
    const Vec2 uv[] = {{0,0},{1,0},{1,1},{0,0},{1,1},{0,1}};
    int i = 0;
    for (Vec3 p : {a, b, c, a, c, d}) mesh.push_back({p, normal, color, uv[i++]});
}

void addWall(std::vector<Vertex>& mesh, float x1, float z1, float x2, float z2,
             float low, float high, Vec3 normal, Vec3 color) {
    addQuad(mesh, {x1, low, z1}, {x2, low, z2}, {x2, high, z2}, {x1, high, z1},
            normal, color);
}

std::vector<Vertex> makeRoom() {
    std::vector<Vertex> mesh;
    const Vec3 ceilingColor{0.85f, 0.87f, 0.86f};
    const Vec3 wallColor{0.78f, 0.82f, 0.79f};
    const Vec3 sideColor{0.69f, 0.76f, 0.78f};
    const Vec3 frameColor{0.34f, 0.39f, 0.39f};

    // A planta e um L: o canto inferior esquerdo recua 0,9 m.
    auto horizontal = [&](float y, Vec3 normal, Vec3 color) {
        addQuad(mesh, {0, y, 0}, {roomWidth, y, 0},
                {roomWidth, y, recessStart}, {0, y, recessStart}, normal, color);
        addQuad(mesh, {recessWidth, y, recessStart},
                {roomWidth, y, recessStart}, {roomWidth, y, roomDepth},
                {recessWidth, y, roomDepth}, normal, color);
    };
    // O piso usa outra malha, para receber sua propria textura.
    horizontal(roomHeight, {0, -1, 0}, ceilingColor);

    addWall(mesh, 0, 0, roomWidth, 0, 0, roomHeight, {0, 0, 1}, wallColor);
    addWall(mesh, recessWidth, roomDepth, roomWidth, roomDepth,
            0, roomHeight, {0, 0, -1}, wallColor);
    addWall(mesh, 0, recessStart, recessWidth, recessStart,
            0, roomHeight, {0, 0, -1}, sideColor);
    addWall(mesh, recessWidth, recessStart, recessWidth, roomDepth,
            0, roomHeight, {1, 0, 0}, sideColor);

    // Abertura real da janela; os mesmos limites orientam a luz natural.
    addWall(mesh, 0, 0, 0, windowZMin, 0, roomHeight, {1, 0, 0}, sideColor);
    addWall(mesh, 0, windowZMin, 0, windowZMax, 0, windowBottom, {1, 0, 0}, sideColor);
    addWall(mesh, 0, windowZMin, 0, windowZMax, windowTop, roomHeight, {1, 0, 0}, sideColor);
    addWall(mesh, 0, windowZMax, 0, recessStart, 0, roomHeight, {1, 0, 0}, sideColor);

    // Parede direita: vao de porta do piso ate 2,20 m.
    addWall(mesh, roomWidth, 0, roomWidth, doorZMin,
            0, roomHeight, {-1, 0, 0}, wallColor);
    addWall(mesh, roomWidth, doorZMin, roomWidth, doorZMax,
            doorTop, roomHeight, {-1, 0, 0}, wallColor);
    addWall(mesh, roomWidth, doorZMax, roomWidth, roomDepth,
            0, roomHeight, {-1, 0, 0}, wallColor);

    // Molduras planas ajudam a reconhecer os dois vaos nesta fase sem esquadrias.
    constexpr float leftFrameX = 0.008f;
    addWall(mesh, leftFrameX, 1.82f, leftFrameX, 1.86f,
            0.82f, 2.13f, {1, 0, 0}, frameColor);
    addWall(mesh, leftFrameX, 3.11f, leftFrameX, 3.15f,
            0.82f, 2.13f, {1, 0, 0}, frameColor);
    addWall(mesh, leftFrameX, 1.85f, leftFrameX, 3.12f,
            0.82f, 0.86f, {1, 0, 0}, frameColor);
    addWall(mesh, leftFrameX, 1.85f, leftFrameX, 3.12f,
            2.09f, 2.13f, {1, 0, 0}, frameColor);

    constexpr float rightFrameX = roomWidth - 0.008f;
    addWall(mesh, rightFrameX, 3.57f, rightFrameX, 3.61f,
            0, 2.23f, {-1, 0, 0}, frameColor);
    addWall(mesh, rightFrameX, 4.54f, rightFrameX, 4.58f,
            0, 2.23f, {-1, 0, 0}, frameColor);
    addWall(mesh, rightFrameX, 3.60f, rightFrameX, 4.55f,
            2.19f, 2.23f, {-1, 0, 0}, frameColor);
    return mesh;
}


} // namespace

MeshData makeMesh(MeshKind kind) {
    if (kind == MeshKind::Shell) return makeRoom();
    MeshData m;
    const Vec3 white{1,1,1};
    if (kind == MeshKind::Floor) {
        addQuad(m, {0,0,0}, {roomWidth,0,0}, {roomWidth,0,recessStart},
                {0,0,recessStart}, {0,1,0}, white);
        addQuad(m, {recessWidth,0,recessStart}, {roomWidth,0,recessStart},
                {roomWidth,0,roomDepth}, {recessWidth,0,roomDepth}, {0,1,0}, white);
        for (auto& v : m) v.uv = {v.position.x / 1.8f, v.position.z / 1.8f};
    } else if (kind == MeshKind::Cube) {
        addQuad(m, {-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f},{0,0,1},white);
        addQuad(m, {.5f,-.5f,-.5f},{-.5f,-.5f,-.5f},{-.5f,.5f,-.5f},{.5f,.5f,-.5f},{0,0,-1},white);
        addQuad(m, {-.5f,-.5f,-.5f},{-.5f,-.5f,.5f},{-.5f,.5f,.5f},{-.5f,.5f,-.5f},{-1,0,0},white);
        addQuad(m, {.5f,-.5f,.5f},{.5f,-.5f,-.5f},{.5f,.5f,-.5f},{.5f,.5f,.5f},{1,0,0},white);
        addQuad(m, {-.5f,.5f,.5f},{.5f,.5f,.5f},{.5f,.5f,-.5f},{-.5f,.5f,-.5f},{0,1,0},white);
        addQuad(m, {-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,-.5f,.5f},{-.5f,-.5f,.5f},{0,-1,0},white);
    } else if (kind == MeshKind::Sphere) {
        constexpr int slices = 48, stacks = 24;
        auto vertex = [&](int x, int y) {
            const float u = float(x)/slices, v = float(y)/stacks;
            const float theta = u*2*pi, phi = v*pi;
            Vec3 n{std::sin(phi)*std::cos(theta), std::cos(phi), std::sin(phi)*std::sin(theta)};
            return Vertex{n*.5f, n, white, {u,v}};
        };
        for (int y=0; y<stacks; ++y) for (int x=0; x<slices; ++x) {
            const Vertex a=vertex(x,y), b=vertex(x+1,y), c=vertex(x+1,y+1), d=vertex(x,y+1);
            if (y>0) m.insert(m.end(), {a,c,b});
            if (y<stacks-1) m.insert(m.end(), {a,d,c});
        }
    } else {
        // Cilindro e cupula: mesma construcao, com raios diferentes no topo.
        const float topRadius = kind == MeshKind::Shade ? .20f : .5f;
        constexpr int slices=32;
        for (int i=0; i<slices; ++i) {
            const float a=2*pi*i/slices, b=2*pi*(i+1)/slices;
            Vec3 na=normalized({std::cos(a), .5f-topRadius, std::sin(a)});
            Vec3 nb=normalized({std::cos(b), .5f-topRadius, std::sin(b)});
            Vertex p{{.5f*std::cos(a),-.5f,.5f*std::sin(a)},na,white,{float(i)/slices,0}};
            Vertex q{{.5f*std::cos(b),-.5f,.5f*std::sin(b)},nb,white,{float(i+1)/slices,0}};
            Vertex r{{topRadius*std::cos(b),.5f,topRadius*std::sin(b)},nb,white,{float(i+1)/slices,1}};
            Vertex s{{topRadius*std::cos(a),.5f,topRadius*std::sin(a)},na,white,{float(i)/slices,1}};
            m.insert(m.end(), {p,r,q,p,s,r});
            auto cap = [&](Vertex v, float sign) {
                v.normal={0,sign,0}; v.uv={v.position.x+.5f,v.position.z+.5f}; return v;
            };
            // A cupula conserva laterais e topo opacos, mas tem abertura inferior.
            if (kind != MeshKind::Shade)
                m.insert(m.end(), {{{0,-.5f,0},{0,-1,0},white,{.5f,.5f}},cap(p,-1),cap(q,-1)});
            m.insert(m.end(), {{{0,.5f,0},{0,1,0},white,{.5f,.5f}},cap(r,1),cap(s,1)});
        }
    }
    return m;
}
