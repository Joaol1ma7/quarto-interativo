#include "behavior.hpp"
#include <iostream>
#include <stdexcept>

void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void advance(Scene& s, Vec3 camera, float seconds) {
    for (int i=0;i<int(seconds*120);++i) updateScene(s,1.f/120,camera);
}
Vec3 panelCenter(const Scene& s, Group group) {
    Part p{"test",MeshKind::Cube,MaterialId::Wood,identity(),group};
    return transformPoint(partModel(s,p),group==Group::Door?Vec3{0,1.1f,.475f}:Vec3{0,.625f,.635f});
}
void pressE(Scene& s, Vec3 camera, Vec3 target, Interaction expected) {
    Vec3 look=normalized(target-camera);
    check(aimedInteraction(s,camera,look)==expected,"Mira nao selecionou o objeto esperado");
    check(applyCommand(s,'E',camera,look),"E nao executou a interacao");
}

int main() {
    try {
        Scene s=makeScene();
        Vec3 doorCamera{7,1.65f,4.2f}, windowCamera{1.8f,1.65f,2.7f}, lampCamera{4.25f,1.65f,3.1f};
        check(aimedInteraction(s,{4,1.65f,4.2f},{1,0,0})==Interaction::None,"Interacao fora de alcance");
        check(!applyCommand(s,'E',doorCamera,{0,0,-1}),"E agiu sem mirar objeto");
        auto occluded=s;
        Vec3 target=panelCenter(s,Group::Door);
        occluded.parts.push_back({"obstaculo",MeshKind::Cube,MaterialId::Wood,
                                  model((doorCamera+target)*.5f,{.5f,.8f,.5f})});
        check(aimedInteraction(occluded,doorCamera,normalized(target-doorCamera))==Interaction::None,"Mira atravessou obstaculo");

        pressE(s,doorCamera,panelCenter(s,Group::Door),Interaction::Door);
        advance(s,doorCamera,.35f);
        check(s.doorAngle<-.1f && s.doorAngle>doorOpenAngle+.1f,"Porta nao animou gradualmente");
        advance(s,doorCamera,2);
        check(std::abs(s.doorAngle-doorOpenAngle)<.001f,"Porta nao abriu ate o limite");
        pressE(s,doorCamera,panelCenter(s,Group::Door),Interaction::Door);
        advance(s,doorCamera,2);
        check(std::abs(s.doorAngle)<.001f,"Porta nao fechou");
        std::cout << "OK: mira, alcance, oclusao, porta abrindo e fechando.\n";

        pressE(s,windowCamera,panelCenter(s,Group::Window),Interaction::Window);
        advance(s,windowCamera,.35f);
        check(s.windowAngle>.1f && s.windowAngle<windowOpenAngle-.1f,"Janela nao animou gradualmente");
        advance(s,windowCamera,2);
        check(std::abs(s.windowAngle-windowOpenAngle)<.001f,"Janela nao abriu");
        pressE(s,windowCamera,panelCenter(s,Group::Window),Interaction::Window);
        advance(s,windowCamera,2);
        check(std::abs(s.windowAngle)<.001f,"Janela nao fechou");
        pressE(s,lampCamera,{4.25f,2.2f,2.3f},Interaction::Lamp);
        check(!s.lampOn,"Luminaria nao desligou");
        pressE(s,{7.1f,1.65f,3.25f},{8.425f,1.30f,3.25f},Interaction::Lamp);
        check(s.lampOn,"Interruptor nao ligou");
        std::cout << "OK: janela abrindo e fechando, luminaria e interruptor.\n";

        s.doorOpen=true;
        Vec3 blocker{7.85f,1.65f,3.90f};
        advance(s,blocker,2);
        check(s.doorBlocked && canWalk(s,blocker.x,blocker.z),"Porta atravessou a camera");
        advance(s,doorCamera,2);
        check(!s.doorBlocked && std::abs(s.doorAngle-doorOpenAngle)<.001f,"Porta nao retomou apos desbloquear");
        s.windowOpen=true; blocker={.65f,1.65f,2.35f};
        advance(s,blocker,2);
        check(s.windowBlocked && canWalk(s,blocker.x,blocker.z),"Janela atravessou a camera");
        advance(s,windowCamera,2);
        check(!s.windowBlocked && std::abs(s.windowAngle-windowOpenAngle)<.001f,"Janela nao retomou");
        std::cout << "OK: colisoes das folhas moveis e retomada da animacao.\n";

        s=makeScene(); Vec3 original=s.ballPosition;
        advance(s,initialCamera,1);
        check(dot(s.ballPosition-original,s.ballPosition-original)<1e-8f,"Bola moveu sem proximidade");
        Vec3 nearCamera{1.8f,1.65f,2.28f};
        advance(s,nearCamera,1);
        float distance=s.ballPosition.x-original.x;
        check(distance>.5f && std::abs(s.ballPosition.z-original.z)<.001f,"Bola nao se afastou da camera");
        check(std::abs(s.ballRotation-distance/ballRadius)<.01f,"Rotacao nao corresponde ao rolamento");
        check(std::abs(s.ballOrientation.values[0]-std::cos(s.ballRotation))<.01f,"Orientacao da bola incorreta");
        advance(s,nearCamera,6);
        check(dot(s.ballVelocity,s.ballVelocity)==0,"Atrito nao parou a bola");
        check(s.ballPosition.y==ballRadius,"Bola saiu do piso");
        for (bool wall : {true,false}) {
            s=makeScene(); s.ballPosition=wall?Vec3{8.22f,.2f,2.7f}:Vec3{4.9f,.2f,.9f};
            s.ballVelocity={3,0,0};
            for (int i=0;i<240;++i) {
                updateScene(s,1.f/120,initialCamera);
                check(clearOfObstacles(s,s.ballPosition.x,s.ballPosition.z,ballRadius,false),"Bola atravessou parede ou movel");
            }
        }
        Scene slow=makeScene(), fast=makeScene();
        for(int i=0;i<90;++i) updateScene(slow,1.f/30,nearCamera);
        for(int i=0;i<360;++i) updateScene(fast,1.f/120,nearCamera);
        check(std::abs(slow.ballPosition.x-fast.ballPosition.x)<.02f,"Fisica depende excessivamente do FPS");
        std::cout << "OK: proximidade, rolamento, atrito, impactos e estabilidade 30/120 FPS.\n";

        s=makeScene(); Vec3 camera{3,1.65f,3};
        for(int i=0;i<200;++i) moveCamera(s,camera,{0,0,1},1.f/60);
        check(camera.z<3.76f && canWalk(s,camera.x,camera.z),"Navegacao atravessou guarda-roupas");
        Vec3 a{4.8f,1.65f,2.7f}, b=a, start=a;
        moveCamera(s,a,{1,0,0},.05f); moveCamera(s,b,{1,0,1},.05f);
        check(std::abs(dot(a-start,a-start)-dot(b-start,b-start))<.0001f,"Diagonal mais rapida");
        Vec3 ambient[3];
        for (int i=0;i<3;++i) {
            check(applyCommand(s,char('1'+i),camera,{0,0,1}),"Tecla de horario nao reconhecida");
            check(static_cast<int>(s.period)==i,"Horario incorreto");
            ambient[i]=lightingFor(s).ambient;
        }
        check(ambient[0].x>ambient[1].x && ambient[1].x>ambient[2].x,"Horarios sem progressao luminosa");
        s.period=DayPeriod::Morning;
        auto closed=lightingFor(s); s.windowAngle=windowOpenAngle; auto opened=lightingFor(s);
        check(opened.natural.x>closed.natural.x*1.5f,"Janela nao altera a luz natural");
        s.lampOn=false; check(dot(lightingFor(s).artificial,lightingFor(s).artificial)==0,"Luz desligada ainda ilumina");
        std::cout << "OK: navegacao, tres horarios, efeito da janela e luz artificial.\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
