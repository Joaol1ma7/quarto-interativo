#include "scene.hpp"
#include "celestial.hpp"

// Pequeno jardim de formas simples. Tudo fica fora do contorno navegavel.
void addLandscape(Scene& cena) {
    using M = MaterialId;
    auto add = [&](const char* nome, MeshKind malha, M material,
                   Vec3 posicao, Vec3 size, unsigned char mask=7) {
        cena.partes.push_back({nome,malha,material,model(posicao,size),
                               Group::Static,true,mask});
    };
    auto box = [&](const char* nome, M material, Vec3 posicao, Vec3 size) {
        add(nome,MeshKind::Cube,material,posicao,size);
    };
    auto sphere = [&](const char* nome, M material, Vec3 posicao, Vec3 size,
                      unsigned char mask=7) {
        add(nome,MeshKind::Sphere,material,posicao,size,mask);
    };

    // A superficie verde passa sob o quarto; o piso interno cobre sua parte visivel.
    box("terreno",M::Grass,{4.25f,-.11f,2.4f},{60.f,.16f,40.f});
    // Dois conjuntos de colinas, um ao fundo de cada abertura.
    for (float lado : {-1.f,1.f}) {
        const float x=lado<0 ? -16.f : 25.f;
        const float z=lado<0 ? 2.5f : 4.1f;
        sphere("colina",M::HillFar,{x,-.8f,z-4.f},{8.f,5.4f,10.f});
        sphere("colina",M::HillNear,{x+lado*2.f,-.9f,z+5.f},{7.f,5.f,10.f});
        sphere("colina",M::HillFar,{x+lado*6.f,-1.2f,z},{8.f,4.8f,9.f});
    }

    // Lajes do caminho saem do vao da porta e continuam pelo gramado.
    for (int i=0;i<10;++i) {
        const float x=8.9f+i*.95f;
        box("caminho",M::Path,{x,-.005f,4.07f},{.78f,.07f,1.03f});
    }

    auto tree = [&](float x,float z,float altura) {
        add("arvore",MeshKind::Cylinder,M::Bark,{x,altura*.30f,z},
            {.22f,altura*.60f,.22f});
        sphere("arvore",M::Leaves,{x,altura*.77f,z},{1.65f,altura*.69f,1.65f});
        sphere("arvore",M::Leaves,{x-.34f,altura*.64f,z+.26f},
               {1.18f,altura*.48f,1.18f});
    };
    tree(-4.0f,1.0f,2.85f);
    tree(-6.6f,4.0f,3.25f);
    tree(-10.f,1.6f,2.65f);
    tree(12.1f,2.45f,3.0f);
    tree(13.2f,5.8f,2.85f);
    tree(17.2f,3.0f,3.4f);

    auto cloud = [&](float x,float y,float z) {
        sphere("nuvem",M::Cloud,{x,y,z},{2.1f,.63f,.85f},3);
        sphere("nuvem",M::Cloud,{x-.48f,y+.15f,z},{1.1f,.68f,.95f},3);
        sphere("nuvem",M::Cloud,{x+.47f,y+.12f,z},{1.35f,.67f,.94f},3);
    };
    cloud(-9.5f,3.9f,6.0f);
    cloud(-13.5f,3.2f,-1.6f);
    cloud(17.5f,3.9f,8.0f);
    cloud(22.f,3.7f,1.0f);

    // Um sol move-se com o horario; a lua ocupa uma unica posicao noturna.
    cena.partes.push_back({"sol",MeshKind::Sphere,M::Sun,
                           scale({1.25f,1.25f,1.25f}),Group::Sun,true,3});
    sphere("lua",M::Moon,celestialVisualPosition(DayPeriod::Night),
           {.95f,.95f,.95f},4);
    for (float lado : {-1.f,1.f}) {
        float x=lado<0 ? -13.f : 20.f;
        float baseZ=lado<0 ? 2.5f : 4.05f;
        for (int i=0;i<13;++i) {
            float z=baseZ+float((i*7)%11-5)*.69f;
            float y=3.1f+float((i*5)%7)*.32f;
            sphere("estrela",M::Star,{x+lado*float(i%3)*.65f,y,z},
                   {.13f,.13f,.13f},4);
        }
    }
}
