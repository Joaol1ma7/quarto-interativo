#pragma once
#include "scene.hpp"

constexpr float doorOpenAngle=-pi/2, windowOpenAngle=80*pi/180;
constexpr float ballRadius=.20f, interactionReach=2.6f;
enum class Interaction { None, Door, Window, Lamp };
struct Lighting { Vec3 sky, ambient, natural, artificial; };

Interaction aimedInteraction(const Scene& scene, Vec3 origin, Vec3 direction);
bool applyCommand(Scene& scene, char key, Vec3 camera, Vec3 lookDirection);
void updateScene(Scene& scene, float dt, Vec3 camera);
void moveCamera(const Scene& scene, Vec3& camera, Vec3 direction, float dt);
bool clearOfObstacles(const Scene& scene, float x, float z, float radius, bool includeWindow);
Collider doorCollider(float angle);
Collider windowCollider(float angle);
Lighting lightingFor(const Scene& scene);
std::string interactionPrompt(const Scene& scene, Interaction target);
std::string statusText(const Scene& scene);
