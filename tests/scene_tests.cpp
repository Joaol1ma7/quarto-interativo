#include "scene.hpp"
#include "lamp.hpp"
#include "celestial.hpp"
#include <iostream>
#include <queue>
#include <set>
#include <stdexcept>

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

bool wallAt(const MeshData& mesh,float x,float y,float z) {
    for (std::size_t i=0;i<mesh.size();i+=3) {
        const Vec3 a=mesh[i].position,b=mesh[i+1].position,c=mesh[i+2].position;
        if (std::abs(a.x-x)>.0001f || std::abs(b.x-x)>.0001f ||
            std::abs(c.x-x)>.0001f) continue;
        const float det=(b.y-a.y)*(c.z-a.z)-(c.y-a.y)*(b.z-a.z);
        if (std::abs(det)<.00001f) continue;
        const float u=((y-a.y)*(c.z-a.z)-(c.y-a.y)*(z-a.z))/det;
        const float v=((b.y-a.y)*(z-a.z)-(y-a.y)*(b.z-a.z))/det;
        if (u>=-.0001f && v>=-.0001f && u+v<=1.0001f) return true;
    }
    return false;
}

int main() {
    try {
        Scene scene=makeScene();
        std::set<std::string> names;
        for (const auto& p : scene.parts) names.insert(p.object);
        for (const char* name : {"cama","guarda-roupas","mesa","cadeira","monitor","teclado",
                                 "porta","janela","luminaria","quadro","bola"})
            require(names.count(name)!=0,"Objeto obrigatorio ausente");
        for (const char* name : {"terreno","colina","arvore","caminho","nuvem","sol","lua","estrela"})
            require(names.count(name)!=0,"Elemento do exterior ausente");
        for (const auto& p : scene.parts) {
            if (!p.outdoor) continue;
            if (p.object=="sol" || p.object=="nuvem") {
                scene.period=DayPeriod::Night;
                require(!visiblePart(scene,p),"Sol ou nuvem visivel de noite");
            }
            if (p.object=="lua" || p.object=="estrela") {
                scene.period=DayPeriod::Morning;
                require(!visiblePart(scene,p),"Lua ou estrela visivel de manha");
                scene.period=DayPeriod::Night;
                require(visiblePart(scene,p),"Lua ou estrela ausente de noite");
            }
        }
        // Cada horario admite um so astro ativo, e o visual segue a fonte
        // usada pelo shader e pelo mapa de sombras.
        constexpr Vec3 roomCenter{roomWidth*.5f,1.f,roomDepth*.5f};
        for (DayPeriod period : {DayPeriod::Morning,DayPeriod::Afternoon,DayPeriod::Night}) {
            scene.period=period;
            int suns=0, moons=0;
            for (const auto& p : scene.parts) {
                if (!visiblePart(scene,p)) continue;
                if (p.object=="sol") ++suns;
                if (p.object=="lua") ++moons;
                if (p.object!="sol" && p.object!="lua") continue;
                const Vec3 visual=transformPoint(partModel(scene,p),{0,0,0});
                const Vec3 expected=celestialVisualPosition(period);
                require(dot(visual-expected,visual-expected)<.0001f,
                        "Astro visual nao acompanha a posicao da luz natural");
                const Vec3 source=naturalPosition(period)-roomCenter;
                require(dot(cross(visual-roomCenter,source),
                            cross(visual-roomCenter,source))<.0001f,
                        "Astro e luz natural apontam para direcoes diferentes");
            }
            require(suns==(period==DayPeriod::Night?0:1) &&
                    moons==(period==DayPeriod::Night?1:0),
                    "Mais de um sol ou lua visivel no mesmo horario");
        }
        scene.period=DayPeriod::Morning;
        for (int kind=0;kind<int(MeshKind::Count);++kind) {
            auto vertices=makeMesh(static_cast<MeshKind>(kind));
            require(!vertices.empty() && vertices.size()%3==0,"Malha sem triangulos completos");
            for (const auto& v : vertices) {
                require(std::isfinite(v.position.x+v.position.y+v.position.z+v.uv.x+v.uv.y),"Vertice/UV invalido");
                require(std::abs(dot(v.normal,v.normal)-1)<.001f,"Normal nao normalizada");
            }
        }
        const auto room=makeMesh(MeshKind::Shell);
        require(!wallAt(room,0,1.5f,2.48f) && wallAt(room,0,2.5f,2.48f),
                "Abertura da janela foi obstruida");
        require(!wallAt(room,roomWidth,1.f,4.f) && wallAt(room,roomWidth,2.5f,4.f),
                "Abertura da porta foi obstruida");
        bool hasCeiling=false;
        for (std::size_t i=0;i<room.size();i+=3)
            hasCeiling |= std::abs(room[i].position.y-roomHeight)<.0001f &&
                          std::abs(room[i+1].position.y-roomHeight)<.0001f &&
                          std::abs(room[i+2].position.y-roomHeight)<.0001f;
        require(hasCeiling,"Teto ausente do mapa geometrico");
        const auto shade=makeMesh(MeshKind::Shade);
        for (std::size_t i=0;i<shade.size();i+=3)
            require(!(shade[i].position.y<-.499f && shade[i+1].position.y<-.499f &&
                      shade[i+2].position.y<-.499f),"Cupula fechou a abertura inferior");
        require(lampEmitter.x==lampAnchor.x && lampEmitter.z==lampAnchor.z &&
                lampEmitter.y<lampShadeY-lampShadeHeight*.5f &&
                lampEmitter.y<lampBulbY-lampBulbHeight*.5f &&
                lampDirection.y==-1.f && lampConeOuterCos<lampConeInnerCos,
                "Emissor fora da abertura da luminaria");
        Mat4 downwardView=lookAt(lampEmitter,lampEmitter+lampDirection);
        for (float value : downwardView.values)
            require(std::isfinite(value),"Matriz vertical da luz invalida");
        Vec3 transformed=transformPoint(translate({1,2,3})*rotateY(pi/2)*scale({2,3,4}),{1,0,0});
        require(std::abs(transformed.x-1)<.001f && std::abs(transformed.y-2)<.001f &&
                std::abs(transformed.z-1)<.001f,"Ordem das transformacoes incorreta");
        Part pivot{"pivo",MeshKind::Cube,MaterialId::Wood,identity(),Group::Door};
        scene.doorAngle=.7f;
        Vec3 hinge=transformPoint(partModel(scene,pivot),{0,0,0});
        require(std::abs(hinge.x-8.46f)<.001f && std::abs(hinge.z-3.60f)<.001f,"Dobradiça mudou de lugar");
        scene.doorAngle=0;

        require(canWalk(scene,initialCamera.x,initialCamera.z),"Camera inicial bloqueada");
        require(!canWalk(scene,.1f,1.f) && !canWalk(scene,.5f,4.f),"Camera atravessa parede/recuo");
        require(!canWalk(scene,-2.f,2.4f) && !canWalk(scene,10.f,4.f),"Exterior ficou navegavel");
        for (const auto& c : scene.colliders)
            require(!canWalk(scene,c.center.x,c.center.y),"Camera atravessa movel");
        require(!canWalk(scene,scene.ballPosition.x,scene.ballPosition.z),"Camera atravessa bola");

        // Busca nos corredores: todos os pontos de observacao precisam ser alcancaveis.
        constexpr int width=86, depth=49;
        bool visited[width][depth]{};
        std::queue<std::pair<int,int>> pending;
        const int startX=static_cast<int>(std::lround(initialCamera.x*10));
        const int startZ=static_cast<int>(std::lround(initialCamera.z*10));
        pending.push({startX,startZ}); visited[startX][startZ]=true;
        const int dx[]={1,-1,0,0}, dz[]={0,0,1,-1};
        while (!pending.empty()) {
            auto [x,z]=pending.front(); pending.pop();
            for (int d=0;d<4;++d) {
                int nx=x+dx[d], nz=z+dz[d];
                if (nx<0 || nz<0 || nx>=width || nz>=depth || visited[nx][nz]) continue;
                if (canWalk(scene,nx*.1f,nz*.1f)) { visited[nx][nz]=true; pending.push({nx,nz}); }
            }
        }
        for (auto [x,z] : {std::pair{32,26},{45,26},{56,25},{60,21},{20,28},{48,33},{71,37}})
            require(visited[x][z],"Corredor bloqueado ate um ponto de observacao");
        std::cout << "OK: objetos, malhas, transformacoes, pivo e corredores navegaveis.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
