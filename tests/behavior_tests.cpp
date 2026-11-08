#include "behavior.hpp"
#include <iostream>
#include <stdexcept>

void check(bool valor, const char* mensagem) { if (!valor) throw std::runtime_error(mensagem); }
void advance(Scene& s, Vec3 camera, float segundos) {
    for (int i=0;i<int(segundos*120);++i) updateScene(s,1.f/120,camera);
}
Vec3 panelCenter(const Scene& s, Group grupo) {
    Part p{"test",MeshKind::Cube,MaterialId::Wood,identity(),grupo};
    return transformPoint(partModel(s,p),grupo==Group::Door?Vec3{0,1.1f,.475f}:Vec3{0,.625f,.635f});
}
void pressE(Scene& s, Vec3 camera, Vec3 alvo, Interaction esperado) {
    Vec3 olhar=normalized(alvo-camera);
    check(aimedInteraction(s,camera,olhar)==esperado,"Mira nao selecionou o objeto esperado");
    check(applyCommand(s,'E',camera,olhar),"E nao executou a interacao");
}

int main() {
    try {
        Scene s=makeScene();
        Vec3 cameraPorta{7,1.65f,4.2f}, cameraJanela{1.8f,1.65f,2.7f}, cameraLuminaria{4.25f,1.65f,3.1f};
        check(aimedInteraction(s,{4,1.65f,4.2f},{1,0,0})==Interaction::None,"Interacao fora de alcance");
        check(!applyCommand(s,'E',cameraPorta,{0,0,-1}),"E agiu sem mirar objeto");
        auto ocluida=s;
        Vec3 alvo=panelCenter(s,Group::Door);
        ocluida.partes.push_back({"obstaculo",MeshKind::Cube,MaterialId::Wood,
                                  model((cameraPorta+alvo)*.5f,{.5f,.8f,.5f})});
        check(aimedInteraction(ocluida,cameraPorta,normalized(alvo-cameraPorta))==Interaction::None,"Mira atravessou obstaculo");

        pressE(s,cameraPorta,panelCenter(s,Group::Door),Interaction::Door);
        advance(s,cameraPorta,.35f);
        check(s.anguloPorta<-.1f && s.anguloPorta>anguloPortaAberta+.1f,"Porta nao animou gradualmente");
        advance(s,cameraPorta,2);
        check(std::abs(s.anguloPorta-anguloPortaAberta)<.001f,"Porta nao abriu ate o limite");
        pressE(s,cameraPorta,panelCenter(s,Group::Door),Interaction::Door);
        advance(s,cameraPorta,2);
        check(std::abs(s.anguloPorta)<.001f,"Porta nao fechou");
        std::cout << "OK: mira, alcance, oclusao, porta abrindo e fechando.\n";

        pressE(s,cameraJanela,panelCenter(s,Group::Window),Interaction::Window);
        advance(s,cameraJanela,.35f);
        check(s.anguloJanela>.1f && s.anguloJanela<anguloJanelaAberta-.1f,"Janela nao animou gradualmente");
        advance(s,cameraJanela,2);
        check(std::abs(s.anguloJanela-anguloJanelaAberta)<.001f,"Janela nao abriu");
        pressE(s,cameraJanela,panelCenter(s,Group::Window),Interaction::Window);
        advance(s,cameraJanela,2);
        check(std::abs(s.anguloJanela)<.001f,"Janela nao fechou");
        pressE(s,cameraLuminaria,{4.25f,2.2f,2.3f},Interaction::Lamp);
        check(!s.luminariaLigada,"Luminaria nao desligou");
        pressE(s,{7.1f,1.65f,3.25f},{8.425f,1.30f,3.25f},Interaction::Lamp);
        check(s.luminariaLigada,"Interruptor nao ligou");
        std::cout << "OK: janela abrindo e fechando, luminaria e interruptor.\n";

        s.portaAberta=true;
        Vec3 bloqueador{7.85f,1.65f,3.90f};
        advance(s,bloqueador,2);
        check(s.portaBloqueada && canWalk(s,bloqueador.x,bloqueador.z),"Porta atravessou a camera");
        advance(s,cameraPorta,2);
        check(!s.portaBloqueada && std::abs(s.anguloPorta-anguloPortaAberta)<.001f,"Porta nao retomou apos desbloquear");
        s.janelaAberta=true; bloqueador={.65f,1.65f,2.35f};
        advance(s,bloqueador,2);
        check(s.janelaBloqueada && canWalk(s,bloqueador.x,bloqueador.z),"Janela atravessou a camera");
        advance(s,cameraJanela,2);
        check(!s.janelaBloqueada && std::abs(s.anguloJanela-anguloJanelaAberta)<.001f,"Janela nao retomou");
        std::cout << "OK: colisoes das folhas moveis e retomada da animacao.\n";

        s=makeScene(); Vec3 original=s.posicaoBola;
        advance(s,cameraInicial,1);
        check(dot(s.posicaoBola-original,s.posicaoBola-original)<1e-8f,"Bola moveu sem proximidade");
        Vec3 cameraProxima{1.8f,1.65f,2.28f};
        advance(s,cameraProxima,1);
        float distancia=s.posicaoBola.x-original.x;
        check(distancia>.5f && std::abs(s.posicaoBola.z-original.z)<.001f,"Bola nao se afastou da camera");
        check(std::abs(s.rotacaoBola-distancia/raioBola)<.01f,"Rotacao nao corresponde ao rolamento");
        check(std::abs(s.orientacaoBola.valores[0]-std::cos(s.rotacaoBola))<.01f,"Orientacao da bola incorreta");
        advance(s,cameraProxima,6);
        check(dot(s.velocidadeBola,s.velocidadeBola)==0,"Atrito nao parou a bola");
        check(s.posicaoBola.y==raioBola,"Bola saiu do piso");
        for (bool parede : {true,false}) {
            s=makeScene(); s.posicaoBola=parede?Vec3{8.22f,.2f,2.7f}:Vec3{4.9f,.2f,.9f};
            s.velocidadeBola={3,0,0};
            for (int i=0;i<240;++i) {
                updateScene(s,1.f/120,cameraInicial);
                check(clearOfObstacles(s,s.posicaoBola.x,s.posicaoBola.z,raioBola,false),"Bola atravessou parede ou movel");
            }
        }
        Scene lento=makeScene(), rapido=makeScene();
        for(int i=0;i<90;++i) updateScene(lento,1.f/30,cameraProxima);
        for(int i=0;i<360;++i) updateScene(rapido,1.f/120,cameraProxima);
        check(std::abs(lento.posicaoBola.x-rapido.posicaoBola.x)<.02f,"Fisica depende excessivamente do FPS");
        std::cout << "OK: proximidade, rolamento, atrito, impactos e estabilidade 30/120 FPS.\n";

        s=makeScene(); Vec3 camera{3,1.65f,3};
        for(int i=0;i<200;++i) moveCamera(s,camera,{0,0,1},1.f/60);
        check(camera.z<3.76f && canWalk(s,camera.x,camera.z),"Navegacao atravessou guarda-roupas");
        Vec3 a{4.8f,1.65f,2.7f}, b=a, inicio=a;
        moveCamera(s,a,{1,0,0},.05f); moveCamera(s,b,{1,0,1},.05f);
        check(std::abs(dot(a-inicio,a-inicio)-dot(b-inicio,b-inicio))<.0001f,"Diagonal mais rapida");
        Vec3 ambiente[3];
        for (int i=0;i<3;++i) {
            check(applyCommand(s,char('1'+i),camera,{0,0,1}),"Tecla de horario nao reconhecida");
            check(static_cast<int>(s.periodo)==i,"Horario incorreto");
            ambiente[i]=lightingFor(s).ambiente;
        }
        check(ambiente[0].x>ambiente[1].x && ambiente[1].x>ambiente[2].x,"Horarios sem progressao luminosa");
        s.periodo=DayPeriod::Morning;
        auto fechada=lightingFor(s); s.anguloJanela=anguloJanelaAberta; auto aberta=lightingFor(s);
        check(aberta.natural.x>fechada.natural.x*1.5f,"Janela nao altera a luz natural");
        s.luminariaLigada=false; check(dot(lightingFor(s).artificial,lightingFor(s).artificial)==0,"Luz desligada ainda ilumina");
        std::cout << "OK: navegacao, tres horarios, efeito da janela e luz artificial.\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
