#include "verification.hpp"
#include "lamp.hpp"
#include <stdexcept>

namespace {
void advance(Scene& s, Vec3 camera, float seconds) {
    for (int i=0;i<int(seconds*120);++i) updateScene(s,1.f/120,camera);
}
Vec3 center(const Scene& s,Group group) {
    Part p{"pivo",MeshKind::Cube,MaterialId::Wood,identity(),group};
    return transformPoint(partModel(s,p),group==Group::Door?Vec3{0,1.1f,.475f}:Vec3{0,.625f,.635f});
}
void interact(Scene& s,Vec3 camera,Vec3 target) {
    if (!applyCommand(s,'E',camera,normalized(target-camera)))
        throw std::runtime_error("Falha na interacao durante captura automatica.");
}
}

std::vector<VerificationFrame> makeVerificationFrames() {
    std::vector<VerificationFrame> frames;
    auto capture=[&](const Scene& scene,Vec3 camera,Vec3 target,const char* name) {
        frames.push_back({scene,camera,target,std::string("verificacao-")+name+".bmp"});
    };
    Scene s=makeScene();
    capture(s,initialCamera,initialTarget,"geral");
    capture(s,{3.2f,1.55f,2.6f},{1.3f,.95f,.6f},"mesa");
    capture(s,{4.5f,1.65f,2.65f},{6.8f,.65f,.85f},"cama");
    capture(s,{5.6f,1.65f,2.5f},{2.8f,1.05f,4.36f},"armario");
    capture(s,{4.8f,1.65f,3.3f},{lampAnchor.x,lampShadeY,lampAnchor.z},"luminaria");

    Vec3 lampCamera{4.25f,1.65f,3.1f}, lamp{lampAnchor.x,lampBulbY,lampAnchor.z};
    Vec3 coneCamera{4.9f,1.65f,3.55f};
    Vec3 coneFloor{lampAnchor.x,0.f,lampAnchor.z};
    Vec3 coneCeiling{lampAnchor.x,2.78f,lampAnchor.z};
    Vec3 windowCamera{1.8f,1.65f,2.7f};
    interact(s,lampCamera,lamp); // Lampada apagada: isola a luz natural.
    applyCommand(s,'1',lampCamera,{0,0,-1});
    capture(s,initialCamera,initialTarget,"manha-fechada");
    capture(s,windowCamera,center(s,Group::Window),"janela-fechada");
    interact(s,windowCamera,center(s,Group::Window));
    advance(s,windowCamera,.5f);
    capture(s,windowCamera,{.45f,1.48f,2.3f},"janela-meio");
    advance(s,windowCamera,2);
    capture(s,windowCamera,center(s,Group::Window),"janela-aberta");
    capture(s,initialCamera,initialTarget,"manha-aberta");
    applyCommand(s,'2',lampCamera,{0,0,-1});
    capture(s,initialCamera,initialTarget,"tarde");
    applyCommand(s,'3',lampCamera,{0,0,-1});
    capture(s,initialCamera,initialTarget,"noite-apagada");
    capture(s,lampCamera,lamp,"lampada-apagada");
    capture(s,coneCamera,coneFloor,"facho-piso-apagada");
    capture(s,coneCamera,coneCeiling,"facho-teto-apagada");
    interact(s,lampCamera,lamp);
    capture(s,initialCamera,initialTarget,"noite-acesa");
    capture(s,lampCamera,lamp,"lampada-acesa");
    capture(s,coneCamera,coneFloor,"facho-piso-acesa");
    capture(s,coneCamera,coneCeiling,"facho-teto-acesa");
    interact(s,{7.1f,1.65f,3.25f},{8.425f,1.3f,3.25f}); // Mesmo circuito no interruptor.
    capture(s,{7.1f,1.65f,3.25f},{8.425f,1.3f,3.25f},"interruptor");
    interact(s,windowCamera,center(s,Group::Window));
    advance(s,windowCamera,2);
    capture(s,windowCamera,center(s,Group::Window),"janela-fechou");
    capture(s,initialCamera,initialTarget,"noite-fechada");

    s=makeScene();
    Vec3 doorCamera{7,1.65f,4.2f}, doorTarget{8.1f,1.1f,3.95f};
    capture(s,doorCamera,doorTarget,"porta-fechada");
    interact(s,doorCamera,center(s,Group::Door));
    advance(s,doorCamera,.5f);
    capture(s,doorCamera,doorTarget,"porta-meio");
    advance(s,doorCamera,2);
    capture(s,doorCamera,doorTarget,"porta-aberta");
    interact(s,doorCamera,center(s,Group::Door));
    advance(s,doorCamera,2);
    capture(s,doorCamera,doorTarget,"porta-fechou");

    // Mesma camera e sol de tarde: a folha deve mover sua sombra no piso.
    s.period=DayPeriod::Afternoon;
    s.lampOn=false;
    Vec3 doorShadowCamera{6.65f,1.65f,2.8f};
    Vec3 doorShadowTarget{8.0f,.15f,3.85f};
    Vec3 bedWallCamera{4.55f,1.65f,2.4f};
    Vec3 bedWallTarget{7.25f,1.05f,.35f};
    capture(s,doorShadowCamera,doorShadowTarget,"sombra-porta-fechada");
    capture(s,bedWallCamera,bedWallTarget,"vazamento-cama-tarde-fechada");
    interact(s,doorCamera,center(s,Group::Door));
    advance(s,doorCamera,2);
    capture(s,doorShadowCamera,doorShadowTarget,"sombra-porta-aberta");
    capture(s,bedWallCamera,bedWallTarget,"vazamento-cama-tarde-aberta");

    // Vidro nao entra no mapa; esquadria acompanha o giro da janela.
    s=makeScene();
    s.lampOn=false;
    Vec3 windowShadowCamera{2.25f,1.65f,3.2f};
    Vec3 windowShadowTarget{.7f,.15f,2.4f};
    capture(s,windowShadowCamera,windowShadowTarget,"sombra-janela-fechada");
    capture(s,{3.45f,1.65f,2.7f},{0,1.35f,2.48f},"vazamento-manha-janela");
    interact(s,windowCamera,center(s,Group::Window));
    advance(s,{3.35f,1.65f,3.3f},2);
    capture(s,windowShadowCamera,windowShadowTarget,"sombra-janela-aberta");

    // Horarios comparados sem alterar camera, objetos ou estado da janela.
    s=makeScene();
    s.lampOn=false;
    capture(s,initialCamera,initialTarget,"sombra-manha");
    s.period=DayPeriod::Afternoon;
    capture(s,initialCamera,initialTarget,"sombra-tarde");

    s=makeScene();
    Vec3 nearBall{1.8f,1.65f,2.28f}, ballTarget=s.ballPosition;
    capture(s,nearBall,ballTarget,"bola-antes");
    advance(s,nearBall,.4f);
    capture(s,nearBall,ballTarget,"bola-rolando");
    advance(s,nearBall,2);
    capture(s,nearBall,ballTarget,"bola-depois");

    // Mesmo jardim observado pelos dois vaos, sem mover a camera entre horarios.
    s=makeScene();
    Vec3 gardenWindowCamera{1.65f,1.65f,2.48f};
    Vec3 gardenDoorCamera{7.05f,1.65f,4.08f};
    interact(s,gardenWindowCamera,center(s,Group::Window));
    advance(s,gardenWindowCamera,2);
    interact(s,gardenDoorCamera,center(s,Group::Door));
    advance(s,gardenDoorCamera,2);
    s.lampOn=false;
    for (char key : {'1','2','3'}) {
        applyCommand(s,key,gardenDoorCamera,{1,0,0});
        const char* period=key=='1'?"manha":key=='2'?"tarde":"noite";
        capture(s,gardenWindowCamera,{-5.f,1.95f,2.52f},
                (std::string("paisagem-janela-")+period).c_str());
        capture(s,gardenDoorCamera,{14.f,1.8f,4.1f},
                (std::string("paisagem-porta-")+period).c_str());
    }

    // Amostras temporarias: mesma forma, vista e posicao em cada condicao de luz.
    s=makeScene();
    const MaterialId samples[]={MaterialId::Wall,MaterialId::Fabric,MaterialId::Wood,
                                MaterialId::Plastic,MaterialId::Metal,MaterialId::Glass};
    for (int i=0;i<6;++i)
        s.parts.push_back({"amostra",MeshKind::Sphere,samples[i],
                           model({2.92f+i*.53f,1.32f,2.20f},{.52f,.52f,.52f})});
    const Vec3 sampleCamera{4.25f,1.65f,3.70f}, sampleTarget{4.25f,1.32f,2.20f};
    if (!canWalk(s,sampleCamera.x,sampleCamera.z))
        throw std::runtime_error("Camera das amostras dentro de um obstaculo.");
    for (char key : {'1','3'}) for (bool on : {false,true}) {
        applyCommand(s,key,sampleCamera,{0,0,-1});
        s.lampOn=on;
        const char* period=key=='1'?"manha":"noite";
        capture(s,sampleCamera,sampleTarget,
                (std::string("materiais-")+period+(on?"-acesa":"-apagada")).c_str());
    }
    return frames;
}
