#include "verification.hpp"
#include "lamp.hpp"
#include <stdexcept>

namespace {
void advance(Scene& s, Vec3 camera, float segundos) {
    for (int i=0;i<int(segundos*120);++i) updateScene(s,1.f/120,camera);
}
Vec3 centro(const Scene& s,Group grupo) {
    Part p{"pivo",MeshKind::Cube,MaterialId::Wood,identity(),grupo};
    return transformPoint(partModel(s,p),grupo==Group::Door?Vec3{0,1.1f,.475f}:Vec3{0,.625f,.635f});
}
void interact(Scene& s,Vec3 camera,Vec3 alvo) {
    if (!applyCommand(s,'E',camera,normalized(alvo-camera)))
        throw std::runtime_error("Falha na interacao durante captura automatica.");
}
}

std::vector<VerificationFrame> makeVerificationFrames() {
    std::vector<VerificationFrame> quadros;
    auto capturar=[&](const Scene& cena,Vec3 camera,Vec3 alvo,const char* nome) {
        quadros.push_back({cena,camera,alvo,std::string("verificacao-")+nome+".bmp"});
    };
    Scene s=makeScene();
    capturar(s,cameraInicial,alvoInicial,"geral");
    capturar(s,{3.2f,1.55f,2.6f},{1.3f,.95f,.6f},"mesa");
    capturar(s,{4.5f,1.65f,2.65f},{6.8f,.65f,.85f},"cama");
    capturar(s,{5.6f,1.65f,2.5f},{2.8f,1.05f,4.36f},"armario");
    capturar(s,{4.8f,1.65f,3.3f},{lampAnchor.x,lampShadeY,lampAnchor.z},"luminaria");

    Vec3 cameraLuminaria{4.25f,1.65f,3.1f}, luminaria{lampAnchor.x,lampBulbY,lampAnchor.z};
    Vec3 cameraCone{4.9f,1.65f,3.55f};
    Vec3 pisoCone{lampAnchor.x,0.f,lampAnchor.z};
    Vec3 tetoCone{lampAnchor.x,2.78f,lampAnchor.z};
    Vec3 cameraJanela{1.8f,1.65f,2.7f};
    interact(s,cameraLuminaria,luminaria); // Lampada apagada: isola a luz natural.
    applyCommand(s,'1',cameraLuminaria,{0,0,-1});
    capturar(s,cameraInicial,alvoInicial,"manha-fechada");
    capturar(s,cameraJanela,centro(s,Group::Window),"janela-fechada");
    interact(s,cameraJanela,centro(s,Group::Window));
    advance(s,cameraJanela,.5f);
    capturar(s,cameraJanela,{.45f,1.48f,2.3f},"janela-meio");
    advance(s,cameraJanela,2);
    capturar(s,cameraJanela,centro(s,Group::Window),"janela-aberta");
    capturar(s,cameraInicial,alvoInicial,"manha-aberta");
    applyCommand(s,'2',cameraLuminaria,{0,0,-1});
    capturar(s,cameraInicial,alvoInicial,"tarde");
    applyCommand(s,'3',cameraLuminaria,{0,0,-1});
    capturar(s,cameraInicial,alvoInicial,"noite-apagada");
    capturar(s,cameraLuminaria,luminaria,"lampada-apagada");
    capturar(s,cameraCone,pisoCone,"facho-piso-apagada");
    capturar(s,cameraCone,tetoCone,"facho-teto-apagada");
    interact(s,cameraLuminaria,luminaria);
    capturar(s,cameraInicial,alvoInicial,"noite-acesa");
    capturar(s,cameraLuminaria,luminaria,"lampada-acesa");
    capturar(s,cameraCone,pisoCone,"facho-piso-acesa");
    capturar(s,cameraCone,tetoCone,"facho-teto-acesa");
    interact(s,{7.1f,1.65f,3.25f},{8.425f,1.3f,3.25f}); // Mesmo circuito no interruptor.
    capturar(s,{7.1f,1.65f,3.25f},{8.425f,1.3f,3.25f},"interruptor");
    interact(s,cameraJanela,centro(s,Group::Window));
    advance(s,cameraJanela,2);
    capturar(s,cameraJanela,centro(s,Group::Window),"janela-fechou");
    capturar(s,cameraInicial,alvoInicial,"noite-fechada");

    s=makeScene();
    Vec3 cameraPorta{7,1.65f,4.2f}, alvoPorta{8.1f,1.1f,3.95f};
    capturar(s,cameraPorta,alvoPorta,"porta-fechada");
    interact(s,cameraPorta,centro(s,Group::Door));
    advance(s,cameraPorta,.5f);
    capturar(s,cameraPorta,alvoPorta,"porta-meio");
    advance(s,cameraPorta,2);
    capturar(s,cameraPorta,alvoPorta,"porta-aberta");
    interact(s,cameraPorta,centro(s,Group::Door));
    advance(s,cameraPorta,2);
    capturar(s,cameraPorta,alvoPorta,"porta-fechou");

    // Mesma camera e sol de tarde: a folha deve mover sua sombra no piso.
    s.periodo=DayPeriod::Afternoon;
    s.luminariaLigada=false;
    Vec3 cameraSombraPorta{6.65f,1.65f,2.8f};
    Vec3 alvoSombraPorta{8.0f,.15f,3.85f};
    Vec3 cameraParedeCama{4.55f,1.65f,2.4f};
    Vec3 alvoParedeCama{7.25f,1.05f,.35f};
    capturar(s,cameraSombraPorta,alvoSombraPorta,"sombra-porta-fechada");
    capturar(s,cameraParedeCama,alvoParedeCama,"vazamento-cama-tarde-fechada");
    interact(s,cameraPorta,centro(s,Group::Door));
    advance(s,cameraPorta,2);
    capturar(s,cameraSombraPorta,alvoSombraPorta,"sombra-porta-aberta");
    capturar(s,cameraParedeCama,alvoParedeCama,"vazamento-cama-tarde-aberta");

    // Vidro nao entra no mapa; esquadria acompanha o giro da janela.
    s=makeScene();
    s.luminariaLigada=false;
    Vec3 cameraSombraJanela{2.25f,1.65f,3.2f};
    Vec3 alvoSombraJanela{.7f,.15f,2.4f};
    capturar(s,cameraSombraJanela,alvoSombraJanela,"sombra-janela-fechada");
    capturar(s,{3.45f,1.65f,2.7f},{0,1.35f,2.48f},"vazamento-manha-janela");
    interact(s,cameraJanela,centro(s,Group::Window));
    advance(s,{3.35f,1.65f,3.3f},2);
    capturar(s,cameraSombraJanela,alvoSombraJanela,"sombra-janela-aberta");

    // Horarios comparados sem alterar camera, objetos ou estado da janela.
    s=makeScene();
    s.luminariaLigada=false;
    capturar(s,cameraInicial,alvoInicial,"sombra-manha");
    s.periodo=DayPeriod::Afternoon;
    capturar(s,cameraInicial,alvoInicial,"sombra-tarde");

    s=makeScene();
    Vec3 pertoBola{1.8f,1.65f,2.28f}, alvoBola=s.posicaoBola;
    capturar(s,pertoBola,alvoBola,"bola-antes");
    advance(s,pertoBola,.4f);
    capturar(s,pertoBola,alvoBola,"bola-rolando");
    advance(s,pertoBola,2);
    capturar(s,pertoBola,alvoBola,"bola-depois");

    // Mesmo jardim observado pelos dois vaos, sem mover a camera entre horarios.
    s=makeScene();
    Vec3 cameraJardimJanela{1.65f,1.65f,2.48f};
    Vec3 cameraJardimPorta{7.05f,1.65f,4.08f};
    interact(s,cameraJardimJanela,centro(s,Group::Window));
    advance(s,cameraJardimJanela,2);
    interact(s,cameraJardimPorta,centro(s,Group::Door));
    advance(s,cameraJardimPorta,2);
    s.luminariaLigada=false;
    for (char tecla : {'1','2','3'}) {
        applyCommand(s,tecla,cameraJardimPorta,{1,0,0});
        const char* periodo=tecla=='1'?"manha":tecla=='2'?"tarde":"noite";
        capturar(s,cameraJardimJanela,{-5.f,1.95f,2.52f},
                (std::string("paisagem-janela-")+periodo).c_str());
        capturar(s,cameraJardimPorta,{14.f,1.8f,4.1f},
                (std::string("paisagem-porta-")+periodo).c_str());
    }

    // Amostras temporarias: mesma forma, vista e posicao em cada condicao de luz.
    s=makeScene();
    const MaterialId amostras[]={MaterialId::Wall,MaterialId::Fabric,MaterialId::Wood,
                                MaterialId::Plastic,MaterialId::Metal,MaterialId::Glass};
    for (int i=0;i<6;++i)
        s.partes.push_back({"amostra",MeshKind::Sphere,amostras[i],
                           model({2.92f+i*.53f,1.32f,2.20f},{.52f,.52f,.52f})});
    const Vec3 cameraAmostra{4.25f,1.65f,3.70f}, alvoAmostra{4.25f,1.32f,2.20f};
    if (!canWalk(s,cameraAmostra.x,cameraAmostra.z))
        throw std::runtime_error("Camera das amostras dentro de um obstaculo.");
    for (char tecla : {'1','3'}) for (bool ligada : {false,true}) {
        applyCommand(s,tecla,cameraAmostra,{0,0,-1});
        s.luminariaLigada=ligada;
        const char* periodo=tecla=='1'?"manha":"noite";
        capturar(s,cameraAmostra,alvoAmostra,
                (std::string("materiais-")+periodo+(ligada?"-acesa":"-apagada")).c_str());
    }
    return quadros;
}
