#include "behavior.hpp"
#include <algorithm>
#include <limits>

namespace {
bool overlaps(const Collider& c, float x, float z, float raio) {
    const float dx=x-c.centro.x, dz=z-c.centro.y;
    const float lx=std::cos(c.guinada)*dx-std::sin(c.guinada)*dz;
    const float lz=std::sin(c.guinada)*dx+std::cos(c.guinada)*dz;
    const float ex=lx-std::clamp(lx,-c.meiaExtensao.x,c.meiaExtensao.x);
    const float ez=lz-std::clamp(lz,-c.meiaExtensao.y,c.meiaExtensao.y);
    return ex*ex+ez*ez < raio*raio;
}

Mat4 inverseAffine(const Mat4& m) {
    const auto& v=m.valores;
    Vec3 a{v[0],v[1],v[2]}, b{v[4],v[5],v[6]}, c{v[8],v[9],v[10]};
    const float det=dot(a,cross(b,c));
    Vec3 r0=cross(b,c)*(1/det), r1=cross(c,a)*(1/det), r2=cross(a,b)*(1/det);
    Vec3 t{v[12],v[13],v[14]};
    Mat4 resultado;
    resultado.valores={r0.x,r1.x,r2.x,0,r0.y,r1.y,r2.y,0,r0.z,r1.z,r2.z,0,
                   -dot(r0,t),-dot(r1,t),-dot(r2,t),1};
    return resultado;
}

float rayBox(Vec3 o, Vec3 d) {
    float proximo=0, distante=std::numeric_limits<float>::max();
    const float origens[]={o.x,o.y,o.z}, direcoes[]={d.x,d.y,d.z};
    for (int i=0;i<3;++i) {
        if (std::abs(direcoes[i])<1e-7f) {
            if (origens[i]<-.5f || origens[i]>.5f) return -1;
        } else {
            float a=(-.5f-origens[i])/direcoes[i], b=(.5f-origens[i])/direcoes[i];
            if (a>b) std::swap(a,b);
            proximo=std::max(proximo,a); distante=std::min(distante,b);
            if (proximo>distante) return -1;
        }
    }
    return distante>0 ? proximo : -1;
}

float rayTriangles(const MeshData& malha, Vec3 o, Vec3 d) {
    float maisProximo=std::numeric_limits<float>::max();
    for (std::size_t i=0;i<malha.size();i+=3) {
        Vec3 a=malha[i].posicao, e1=malha[i+1].posicao-a, e2=malha[i+2].posicao-a;
        Vec3 h=cross(d,e2);
        float det=dot(e1,h);
        if (std::abs(det)<1e-8f) continue;
        Vec3 s=o-a;
        float u=dot(s,h)/det;
        if (u<0 || u>1) continue;
        Vec3 q=cross(s,e1);
        float v=dot(d,q)/det;
        if (v<0 || u+v>1) continue;
        float t=dot(e2,q)/det;
        if (t>0 && t<maisProximo) maisProximo=t;
    }
    return maisProximo;
}

Mat4 roll(Vec3 deslocamento) {
    const float distancia=std::sqrt(dot(deslocamento,deslocamento));
    Vec3 a=normalized(cross({0,1,0},deslocamento));
    float c=std::cos(distancia/raioBola), s=std::sin(distancia/raioBola), k=1-c;
    Mat4 m=identity();
    m.valores[0]=c+a.x*a.x*k; m.valores[4]=a.x*a.y*k-a.z*s; m.valores[8]=a.x*a.z*k+a.y*s;
    m.valores[1]=a.y*a.x*k+a.z*s; m.valores[5]=c+a.y*a.y*k; m.valores[9]=a.y*a.z*k-a.x*s;
    m.valores[2]=a.z*a.x*k-a.y*s; m.valores[6]=a.z*a.y*k+a.x*s; m.valores[10]=c+a.z*a.z*k;
    return m;
}

float approach(float valor, float alvo, float passo) {
    return valor+std::clamp(alvo-valor,-passo,passo);
}
} // namespace

Collider doorCollider(float angulo) {
    return {{8.46f+.475f*std::sin(angulo),3.60f+.475f*std::cos(angulo)}, {.095f,.48f},angulo};
}
Collider windowCollider(float angulo) {
    return {{.04f+.635f*std::sin(angulo),1.85f+.635f*std::cos(angulo)}, {.065f,.635f},angulo};
}

bool clearOfObstacles(const Scene& cena, float x, float z, float raio, bool incluirJanela) {
    if (x<raio || x>larguraQuarto-raio || z<raio || z>profundidadeQuarto-raio) return false;
    if (x<larguraRecuo+raio && z>inicioRecuo-raio) return false;
    for (const auto& c : cena.colisores) if (overlaps(c,x,z,raio)) return false;
    if (overlaps(doorCollider(cena.anguloPorta),x,z,raio)) return false;
    if (incluirJanela && overlaps(windowCollider(cena.anguloJanela),x,z,raio)) return false;
    return true;
}

bool canWalk(const Scene& cena, float x, float z) {
    if (!clearOfObstacles(cena,x,z,raioCamera,true)) return false;
    float dx=x-cena.posicaoBola.x, dz=z-cena.posicaoBola.z;
    return dx*dx+dz*dz >= (raioBola+raioCamera)*(raioBola+raioCamera);
}

void moveCamera(const Scene& cena, Vec3& camera, Vec3 direcao, float dt) {
    direcao.y=0;
    if (dot(direcao,direcao)<1e-8f || dt<=0) return;
    Vec3 deslocamento=normalized(direcao)*(2.8f*std::min(dt,.05f));
    int passos=std::max(1,int(std::ceil(std::sqrt(dot(deslocamento,deslocamento))/.04f)));
    deslocamento=deslocamento*(1.f/passos);
    for (int i=0;i<passos;++i) {
        if (canWalk(cena,camera.x+deslocamento.x,camera.z)) camera.x+=deslocamento.x;
        if (canWalk(cena,camera.x,camera.z+deslocamento.z)) camera.z+=deslocamento.z;
    }
}

Interaction aimedInteraction(const Scene& cena, Vec3 origem, Vec3 direcao) {
    if (dot(direcao,direcao)<1e-8f) return Interaction::None;
    direcao=normalized(direcao);
    static const std::array<MeshData,static_cast<int>(MeshKind::Count)> malhas=[] {
        std::array<MeshData,static_cast<int>(MeshKind::Count)> resultado;
        for (int i=0;i<int(MeshKind::Count);++i) resultado[i]=makeMesh(static_cast<MeshKind>(i));
        return resultado;
    }();
    float maisProximo=alcanceInteracao;
    Interaction selecionada=Interaction::None;
    for (const auto& parte : cena.partes) {
        if (parte.externo || !visiblePart(cena,parte)) continue;
        Mat4 inversa=inverseAffine(partModel(cena,parte));
        Vec3 o=transformPoint(inversa,origem);
        Vec3 d=transformPoint(inversa,origem+direcao)-o;
        float t=-1;
        if (parte.malha==MeshKind::Cube) t=rayBox(o,d);
        else if (parte.malha==MeshKind::Sphere) {
            float a=dot(d,d), b=dot(o,d), c=dot(o,o)-.25f, discriminante=b*b-a*c;
            if (discriminante>=0) {
                t=(-b-std::sqrt(discriminante))/a;
                if (t<0) t=(-b+std::sqrt(discriminante))/a;
            }
        } else if (parte.malha==MeshKind::Shell || parte.malha==MeshKind::Floor || rayBox(o,d)>=0) {
            t=rayTriangles(malhas[static_cast<int>(parte.malha)],o,d);
        }
        if (t<0 || t>=maisProximo) continue;
        maisProximo=t;
        selecionada=Interaction::None;
        if (parte.objeto=="porta") selecionada=Interaction::Door;
        if (parte.objeto=="janela") selecionada=Interaction::Window;
        if (parte.objeto=="luminaria" || parte.objeto=="interruptor") selecionada=Interaction::Lamp;
    }
    return selecionada;
}

bool applyCommand(Scene& cena, char tecla, Vec3 camera, Vec3 direcaoOlhar) {
    if (tecla>='1' && tecla<='3') {
        cena.periodo=static_cast<DayPeriod>(tecla-'1');
        return true;
    }
    if (tecla!='E') return false;
    switch (aimedInteraction(cena,camera,direcaoOlhar)) {
    case Interaction::Door: cena.portaAberta=!cena.portaAberta; return true;
    case Interaction::Window: cena.janelaAberta=!cena.janelaAberta; return true;
    case Interaction::Lamp: cena.luminariaLigada=!cena.luminariaLigada; return true;
    default: return false;
    }
}

void updateScene(Scene& cena, float dt, Vec3 camera) {
    if (dt<=0) return;
    dt=std::min(dt,.05f);
    Vec3 afastamento=cena.posicaoBola-camera; afastamento.y=0;
    float distancia=std::sqrt(dot(afastamento,afastamento));
    if (!cena.bolaProxima && distancia<.80f) {
        cena.velocidadeBola=(distancia>.001f ? afastamento*(1/distancia) : Vec3{1,0,0})*1.65f;
        cena.bolaProxima=true;
    } else if (distancia>1.05f) cena.bolaProxima=false;

    const int passos=std::max(1,int(std::ceil(dt*120)));
    const float passo=dt/passos;
    cena.portaBloqueada=cena.janelaBloqueada=false;
    for (int i=0;i<passos;++i) {
        float proximaPorta=approach(cena.anguloPorta,cena.portaAberta?anguloPortaAberta:0,1.35f*passo);
        Collider porta=doorCollider(proximaPorta);
        if (overlaps(porta,camera.x,camera.z,raioCamera+.015f) ||
            overlaps(porta,cena.posicaoBola.x,cena.posicaoBola.z,raioBola+.01f))
            cena.portaBloqueada=proximaPorta!=cena.anguloPorta;
        else cena.anguloPorta=proximaPorta;
        float proximaJanela=approach(cena.anguloJanela,cena.janelaAberta?anguloJanelaAberta:0,1.15f*passo);
        if (overlaps(windowCollider(proximaJanela),camera.x,camera.z,raioCamera+.015f))
            cena.janelaBloqueada=proximaJanela!=cena.anguloJanela;
        else cena.anguloJanela=proximaJanela;

        Vec3 antes=cena.posicaoBola;
        auto bolaLivre=[&](float x,float z) {
            float dx=x-camera.x, dz=z-camera.z;
            return clearOfObstacles(cena,x,z,raioBola,false) &&
                   dx*dx+dz*dz >= (raioCamera+raioBola)*(raioCamera+raioBola);
        };
        float x=cena.posicaoBola.x+cena.velocidadeBola.x*passo;
        if (bolaLivre(x,cena.posicaoBola.z)) cena.posicaoBola.x=x;
        else cena.velocidadeBola.x*=-.45f;
        float z=cena.posicaoBola.z+cena.velocidadeBola.z*passo;
        if (bolaLivre(cena.posicaoBola.x,z)) cena.posicaoBola.z=z;
        else cena.velocidadeBola.z*=-.45f;
        Vec3 percorrido=cena.posicaoBola-antes;
        const float length=std::sqrt(dot(percorrido,percorrido));
        if (length>1e-7f) {
            cena.orientacaoBola=roll(percorrido)*cena.orientacaoBola;
            cena.rotacaoBola+=length/raioBola;
        }
        cena.velocidadeBola=cena.velocidadeBola*std::exp(-1.45f*passo);
        if (dot(cena.velocidadeBola,cena.velocidadeBola)<.0004f) cena.velocidadeBola={0,0,0};
    }
}

Lighting lightingFor(const Scene& cena) {
    Lighting l;
    const float abertura=std::clamp(cena.anguloJanela/anguloJanelaAberta,0.f,1.f);
    const float transmissao=.52f+.48f*abertura;
    const float transmissaoAmbiente=.82f+.18f*abertura;
    if (cena.periodo==DayPeriod::Morning) {
        l.ceu={.48f,.73f,.94f}; l.ambiente=Vec3{.58f,.60f,.61f}*transmissaoAmbiente;
        l.natural=Vec3{1.05f,1.02f,.94f}*transmissao;
    } else if (cena.periodo==DayPeriod::Afternoon) {
        l.ceu={.65f,.39f,.23f}; l.ambiente=Vec3{.42f,.32f,.25f}*transmissaoAmbiente;
        l.natural=Vec3{.78f,.44f,.22f}*transmissao;
    } else {
        l.ceu={.012f,.023f,.055f}; l.ambiente=Vec3{.045f,.060f,.095f}*transmissaoAmbiente;
        l.natural=Vec3{.035f,.055f,.10f}*transmissao;
    }
    l.artificial=cena.luminariaLigada ? Vec3{1.12f,.99f,.76f} : Vec3{0,0,0};
    return l;
}

std::string interactionPrompt(const Scene& cena, Interaction alvo) {
    if (alvo==Interaction::Door) {
        if (cena.portaBloqueada) return "E - INVERTER PORTA / AFASTE-SE";
        return cena.portaAberta?"E - FECHAR PORTA":"E - ABRIR PORTA";
    }
    if (alvo==Interaction::Window) {
        if (cena.janelaBloqueada) return "E - INVERTER JANELA / AFASTE-SE";
        return cena.janelaAberta?"E - FECHAR JANELA":"E - ABRIR JANELA";
    }
    if (alvo==Interaction::Lamp) return cena.luminariaLigada?"E - DESLIGAR LUZ":"E - LIGAR LUZ";
    return "";
}
std::string statusText(const Scene& cena) {
    const char* periods[]={"MANHA","TARDE","NOITE"};
    return std::string("1/2/3 ")+periods[static_cast<int>(cena.periodo)]+" | LUZ "+
           (cena.luminariaLigada?"LIGADA":"DESLIGADA");
}
