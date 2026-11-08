#pragma once
#include "scene.hpp"

constexpr float anguloPortaAberta=-pi/2, anguloJanelaAberta=80*pi/180;
constexpr float raioBola=.20f, alcanceInteracao=2.6f;
enum class Interaction { None, Door, Window, Lamp };
struct Lighting { Vec3 ceu, ambiente, natural, artificial; };

Interaction aimedInteraction(const Scene& cena, Vec3 origem, Vec3 direcao);
bool applyCommand(Scene& cena, char tecla, Vec3 camera, Vec3 direcaoOlhar);
void updateScene(Scene& cena, float dt, Vec3 camera);
void moveCamera(const Scene& cena, Vec3& camera, Vec3 direcao, float dt);
bool clearOfObstacles(const Scene& cena, float x, float z, float raio, bool incluirJanela);
Collider doorCollider(float angulo);
Collider windowCollider(float angulo);
Lighting lightingFor(const Scene& cena);
std::string interactionPrompt(const Scene& cena, Interaction alvo);
std::string statusText(const Scene& cena);
