#include "scene.hpp"
#include "lamp.hpp"
#include "celestial.hpp"
#include <iostream>
#include <queue>
#include <set>
#include <stdexcept>

void require(bool condicao, const char* mensagem) {
    if (!condicao) throw std::runtime_error(mensagem);
}

bool wallAt(const MeshData& malha,float x,float y,float z) {
    for (std::size_t i=0;i<malha.size();i+=3) {
        const Vec3 a=malha[i].posicao,b=malha[i+1].posicao,c=malha[i+2].posicao;
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
        Scene cena=makeScene();
        std::set<std::string> nomes;
        for (const auto& p : cena.partes) nomes.insert(p.objeto);
        for (const char* nome : {"cama","guarda-roupas","mesa","cadeira","monitor","teclado",
                                 "porta","janela","luminaria","quadro","bola"})
            require(nomes.count(nome)!=0,"Objeto obrigatorio ausente");
        for (const char* nome : {"terreno","colina","arvore","caminho","nuvem","sol","lua","estrela"})
            require(nomes.count(nome)!=0,"Elemento do exterior ausente");
        for (const auto& p : cena.partes) {
            if (!p.externo) continue;
            if (p.objeto=="sol" || p.objeto=="nuvem") {
                cena.periodo=DayPeriod::Night;
                require(!visiblePart(cena,p),"Sol ou nuvem visivel de noite");
            }
            if (p.objeto=="lua" || p.objeto=="estrela") {
                cena.periodo=DayPeriod::Morning;
                require(!visiblePart(cena,p),"Lua ou estrela visivel de manha");
                cena.periodo=DayPeriod::Night;
                require(visiblePart(cena,p),"Lua ou estrela ausente de noite");
            }
        }
        // Cada horario admite um so astro ativo, e o visual segue a fonte
        // usada pelo sombreador e pelo mapa de sombras.
        constexpr Vec3 centroQuarto{larguraQuarto*.5f,1.f,profundidadeQuarto*.5f};
        for (DayPeriod periodo : {DayPeriod::Morning,DayPeriod::Afternoon,DayPeriod::Night}) {
            cena.periodo=periodo;
            int sois=0, luas=0;
            for (const auto& p : cena.partes) {
                if (!visiblePart(cena,p)) continue;
                if (p.objeto=="sol") ++sois;
                if (p.objeto=="lua") ++luas;
                if (p.objeto!="sol" && p.objeto!="lua") continue;
                const Vec3 visual=transformPoint(partModel(cena,p),{0,0,0});
                const Vec3 esperado=celestialVisualPosition(periodo);
                require(dot(visual-esperado,visual-esperado)<.0001f,
                        "Astro visual nao acompanha a posicao da luz natural");
                const Vec3 fonte=naturalPosition(periodo)-centroQuarto;
                require(dot(cross(visual-centroQuarto,fonte),
                            cross(visual-centroQuarto,fonte))<.0001f,
                        "Astro e luz natural apontam para direcoes diferentes");
            }
            require(sois==(periodo==DayPeriod::Night?0:1) &&
                    luas==(periodo==DayPeriod::Night?1:0),
                    "Mais de um sol ou lua visivel no mesmo horario");
        }
        cena.periodo=DayPeriod::Morning;
        for (int tipo=0;tipo<int(MeshKind::Count);++tipo) {
            auto vertices=makeMesh(static_cast<MeshKind>(tipo));
            require(!vertices.empty() && vertices.size()%3==0,"Malha sem triangulos completos");
            for (const auto& v : vertices) {
                require(std::isfinite(v.posicao.x+v.posicao.y+v.posicao.z+v.uv.x+v.uv.y),"Vertice/UV invalido");
                require(std::abs(dot(v.normal,v.normal)-1)<.001f,"Normal nao normalizada");
            }
        }
        const auto quarto=makeMesh(MeshKind::Shell);
        require(!wallAt(quarto,0,1.5f,2.48f) && wallAt(quarto,0,2.5f,2.48f),
                "Abertura da janela foi obstruida");
        require(!wallAt(quarto,larguraQuarto,1.f,4.f) && wallAt(quarto,larguraQuarto,2.5f,4.f),
                "Abertura da porta foi obstruida");
        bool temTeto=false;
        for (std::size_t i=0;i<quarto.size();i+=3)
            temTeto |= std::abs(quarto[i].posicao.y-alturaQuarto)<.0001f &&
                          std::abs(quarto[i+1].posicao.y-alturaQuarto)<.0001f &&
                          std::abs(quarto[i+2].posicao.y-alturaQuarto)<.0001f;
        require(temTeto,"Teto ausente do mapa geometrico");
        const auto cupula=makeMesh(MeshKind::Shade);
        for (std::size_t i=0;i<cupula.size();i+=3)
            require(!(cupula[i].posicao.y<-.499f && cupula[i+1].posicao.y<-.499f &&
                      cupula[i+2].posicao.y<-.499f),"Cupula fechou a abertura inferior");
        require(lampEmitter.x==lampAnchor.x && lampEmitter.z==lampAnchor.z &&
                lampEmitter.y<lampShadeY-lampShadeHeight*.5f &&
                lampEmitter.y<lampBulbY-lampBulbHeight*.5f &&
                lampDirection.y==-1.f && lampConeOuterCos<lampConeInnerCos,
                "Emissor fora da abertura da luminaria");
        Mat4 visaoParaBaixo=lookAt(lampEmitter,lampEmitter+lampDirection);
        for (float valor : visaoParaBaixo.valores)
            require(std::isfinite(valor),"Matriz vertical da luz invalida");
        Vec3 transformado=transformPoint(translate({1,2,3})*rotateY(pi/2)*scale({2,3,4}),{1,0,0});
        require(std::abs(transformado.x-1)<.001f && std::abs(transformado.y-2)<.001f &&
                std::abs(transformado.z-1)<.001f,"Ordem das transformacoes incorreta");
        Part pivo{"pivo",MeshKind::Cube,MaterialId::Wood,identity(),Group::Door};
        cena.anguloPorta=.7f;
        Vec3 dobradica=transformPoint(partModel(cena,pivo),{0,0,0});
        require(std::abs(dobradica.x-8.46f)<.001f && std::abs(dobradica.z-3.60f)<.001f,"Dobradiça mudou de lugar");
        cena.anguloPorta=0;

        require(canWalk(cena,cameraInicial.x,cameraInicial.z),"Camera inicial bloqueada");
        require(!canWalk(cena,.1f,1.f) && !canWalk(cena,.5f,4.f),"Camera atravessa parede/recuo");
        require(!canWalk(cena,-2.f,2.4f) && !canWalk(cena,10.f,4.f),"Exterior ficou navegavel");
        for (const auto& c : cena.colisores)
            require(!canWalk(cena,c.centro.x,c.centro.y),"Camera atravessa movel");
        require(!canWalk(cena,cena.posicaoBola.x,cena.posicaoBola.z),"Camera atravessa bola");

        // Busca nos corredores: todos os pontos de observacao precisam ser alcancaveis.
        constexpr int largura=86, profundidade=49;
        bool visitado[largura][profundidade]{};
        std::queue<std::pair<int,int>> pendente;
        const int inicioX=static_cast<int>(std::lround(cameraInicial.x*10));
        const int inicioZ=static_cast<int>(std::lround(cameraInicial.z*10));
        pendente.push({inicioX,inicioZ}); visitado[inicioX][inicioZ]=true;
        const int dx[]={1,-1,0,0}, dz[]={0,0,1,-1};
        while (!pendente.empty()) {
            auto [x,z]=pendente.front(); pendente.pop();
            for (int d=0;d<4;++d) {
                int nx=x+dx[d], nz=z+dz[d];
                if (nx<0 || nz<0 || nx>=largura || nz>=profundidade || visitado[nx][nz]) continue;
                if (canWalk(cena,nx*.1f,nz*.1f)) { visitado[nx][nz]=true; pendente.push({nx,nz}); }
            }
        }
        for (auto [x,z] : {std::pair{32,26},{45,26},{56,25},{60,21},{20,28},{48,33},{71,37}})
            require(visitado[x][z],"Corredor bloqueado ate um ponto de observacao");
        std::cout << "OK: objetos, malhas, transformacoes, pivo e corredores navegaveis.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
