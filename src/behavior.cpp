#include "behavior.hpp"
#include <algorithm>
#include <limits>

namespace {
bool overlaps(const Collider& c, float x, float z, float radius) {
    const float dx=x-c.center.x, dz=z-c.center.y;
    const float lx=std::cos(c.yaw)*dx-std::sin(c.yaw)*dz;
    const float lz=std::sin(c.yaw)*dx+std::cos(c.yaw)*dz;
    const float ex=lx-std::clamp(lx,-c.halfSize.x,c.halfSize.x);
    const float ez=lz-std::clamp(lz,-c.halfSize.y,c.halfSize.y);
    return ex*ex+ez*ez < radius*radius;
}

Mat4 inverseAffine(const Mat4& m) {
    const auto& v=m.values;
    Vec3 a{v[0],v[1],v[2]}, b{v[4],v[5],v[6]}, c{v[8],v[9],v[10]};
    const float det=dot(a,cross(b,c));
    Vec3 r0=cross(b,c)*(1/det), r1=cross(c,a)*(1/det), r2=cross(a,b)*(1/det);
    Vec3 t{v[12],v[13],v[14]};
    Mat4 result;
    result.values={r0.x,r1.x,r2.x,0,r0.y,r1.y,r2.y,0,r0.z,r1.z,r2.z,0,
                   -dot(r0,t),-dot(r1,t),-dot(r2,t),1};
    return result;
}

float rayBox(Vec3 o, Vec3 d) {
    float near=0, far=std::numeric_limits<float>::max();
    const float origins[]={o.x,o.y,o.z}, dirs[]={d.x,d.y,d.z};
    for (int i=0;i<3;++i) {
        if (std::abs(dirs[i])<1e-7f) {
            if (origins[i]<-.5f || origins[i]>.5f) return -1;
        } else {
            float a=(-.5f-origins[i])/dirs[i], b=(.5f-origins[i])/dirs[i];
            if (a>b) std::swap(a,b);
            near=std::max(near,a); far=std::min(far,b);
            if (near>far) return -1;
        }
    }
    return far>0 ? near : -1;
}

float rayTriangles(const MeshData& mesh, Vec3 o, Vec3 d) {
    float nearest=std::numeric_limits<float>::max();
    for (std::size_t i=0;i<mesh.size();i+=3) {
        Vec3 a=mesh[i].position, e1=mesh[i+1].position-a, e2=mesh[i+2].position-a;
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
        if (t>0 && t<nearest) nearest=t;
    }
    return nearest;
}

Mat4 roll(Vec3 displacement) {
    const float distance=std::sqrt(dot(displacement,displacement));
    Vec3 a=normalized(cross({0,1,0},displacement));
    float c=std::cos(distance/ballRadius), s=std::sin(distance/ballRadius), k=1-c;
    Mat4 m=identity();
    m.values[0]=c+a.x*a.x*k; m.values[4]=a.x*a.y*k-a.z*s; m.values[8]=a.x*a.z*k+a.y*s;
    m.values[1]=a.y*a.x*k+a.z*s; m.values[5]=c+a.y*a.y*k; m.values[9]=a.y*a.z*k-a.x*s;
    m.values[2]=a.z*a.x*k-a.y*s; m.values[6]=a.z*a.y*k+a.x*s; m.values[10]=c+a.z*a.z*k;
    return m;
}

float approach(float value, float target, float step) {
    return value+std::clamp(target-value,-step,step);
}
} // namespace

Collider doorCollider(float angle) {
    return {{8.46f+.475f*std::sin(angle),3.60f+.475f*std::cos(angle)}, {.095f,.48f},angle};
}
Collider windowCollider(float angle) {
    return {{.04f+.635f*std::sin(angle),1.85f+.635f*std::cos(angle)}, {.065f,.635f},angle};
}

bool clearOfObstacles(const Scene& scene, float x, float z, float radius, bool includeWindow) {
    if (x<radius || x>roomWidth-radius || z<radius || z>roomDepth-radius) return false;
    if (x<recessWidth+radius && z>recessStart-radius) return false;
    for (const auto& c : scene.colliders) if (overlaps(c,x,z,radius)) return false;
    if (overlaps(doorCollider(scene.doorAngle),x,z,radius)) return false;
    if (includeWindow && overlaps(windowCollider(scene.windowAngle),x,z,radius)) return false;
    return true;
}

bool canWalk(const Scene& scene, float x, float z) {
    if (!clearOfObstacles(scene,x,z,cameraRadius,true)) return false;
    float dx=x-scene.ballPosition.x, dz=z-scene.ballPosition.z;
    return dx*dx+dz*dz >= (ballRadius+cameraRadius)*(ballRadius+cameraRadius);
}

void moveCamera(const Scene& scene, Vec3& camera, Vec3 direction, float dt) {
    direction.y=0;
    if (dot(direction,direction)<1e-8f || dt<=0) return;
    Vec3 delta=normalized(direction)*(2.8f*std::min(dt,.05f));
    int steps=std::max(1,int(std::ceil(std::sqrt(dot(delta,delta))/.04f)));
    delta=delta*(1.f/steps);
    for (int i=0;i<steps;++i) {
        if (canWalk(scene,camera.x+delta.x,camera.z)) camera.x+=delta.x;
        if (canWalk(scene,camera.x,camera.z+delta.z)) camera.z+=delta.z;
    }
}

Interaction aimedInteraction(const Scene& scene, Vec3 origin, Vec3 direction) {
    if (dot(direction,direction)<1e-8f) return Interaction::None;
    direction=normalized(direction);
    static const std::array<MeshData,static_cast<int>(MeshKind::Count)> meshes=[] {
        std::array<MeshData,static_cast<int>(MeshKind::Count)> result;
        for (int i=0;i<int(MeshKind::Count);++i) result[i]=makeMesh(static_cast<MeshKind>(i));
        return result;
    }();
    float nearest=interactionReach;
    Interaction selected=Interaction::None;
    for (const auto& part : scene.parts) {
        if (part.outdoor || !visiblePart(scene,part)) continue;
        Mat4 inverse=inverseAffine(partModel(scene,part));
        Vec3 o=transformPoint(inverse,origin);
        Vec3 d=transformPoint(inverse,origin+direction)-o;
        float t=-1;
        if (part.mesh==MeshKind::Cube) t=rayBox(o,d);
        else if (part.mesh==MeshKind::Sphere) {
            float a=dot(d,d), b=dot(o,d), c=dot(o,o)-.25f, discriminant=b*b-a*c;
            if (discriminant>=0) {
                t=(-b-std::sqrt(discriminant))/a;
                if (t<0) t=(-b+std::sqrt(discriminant))/a;
            }
        } else if (part.mesh==MeshKind::Shell || part.mesh==MeshKind::Floor || rayBox(o,d)>=0) {
            t=rayTriangles(meshes[static_cast<int>(part.mesh)],o,d);
        }
        if (t<0 || t>=nearest) continue;
        nearest=t;
        selected=Interaction::None;
        if (part.object=="porta") selected=Interaction::Door;
        if (part.object=="janela") selected=Interaction::Window;
        if (part.object=="luminaria" || part.object=="interruptor") selected=Interaction::Lamp;
    }
    return selected;
}

bool applyCommand(Scene& scene, char key, Vec3 camera, Vec3 lookDirection) {
    if (key>='1' && key<='3') {
        scene.period=static_cast<DayPeriod>(key-'1');
        return true;
    }
    if (key!='E') return false;
    switch (aimedInteraction(scene,camera,lookDirection)) {
    case Interaction::Door: scene.doorOpen=!scene.doorOpen; return true;
    case Interaction::Window: scene.windowOpen=!scene.windowOpen; return true;
    case Interaction::Lamp: scene.lampOn=!scene.lampOn; return true;
    default: return false;
    }
}

void updateScene(Scene& scene, float dt, Vec3 camera) {
    if (dt<=0) return;
    dt=std::min(dt,.05f);
    Vec3 away=scene.ballPosition-camera; away.y=0;
    float distance=std::sqrt(dot(away,away));
    if (!scene.ballNear && distance<.80f) {
        scene.ballVelocity=(distance>.001f ? away*(1/distance) : Vec3{1,0,0})*1.65f;
        scene.ballNear=true;
    } else if (distance>1.05f) scene.ballNear=false;

    const int steps=std::max(1,int(std::ceil(dt*120)));
    const float step=dt/steps;
    scene.doorBlocked=scene.windowBlocked=false;
    for (int i=0;i<steps;++i) {
        float nextDoor=approach(scene.doorAngle,scene.doorOpen?doorOpenAngle:0,1.35f*step);
        Collider door=doorCollider(nextDoor);
        if (overlaps(door,camera.x,camera.z,cameraRadius+.015f) ||
            overlaps(door,scene.ballPosition.x,scene.ballPosition.z,ballRadius+.01f))
            scene.doorBlocked=nextDoor!=scene.doorAngle;
        else scene.doorAngle=nextDoor;
        float nextWindow=approach(scene.windowAngle,scene.windowOpen?windowOpenAngle:0,1.15f*step);
        if (overlaps(windowCollider(nextWindow),camera.x,camera.z,cameraRadius+.015f))
            scene.windowBlocked=nextWindow!=scene.windowAngle;
        else scene.windowAngle=nextWindow;

        Vec3 before=scene.ballPosition;
        auto freeBall=[&](float x,float z) {
            float dx=x-camera.x, dz=z-camera.z;
            return clearOfObstacles(scene,x,z,ballRadius,false) &&
                   dx*dx+dz*dz >= (cameraRadius+ballRadius)*(cameraRadius+ballRadius);
        };
        float x=scene.ballPosition.x+scene.ballVelocity.x*step;
        if (freeBall(x,scene.ballPosition.z)) scene.ballPosition.x=x;
        else scene.ballVelocity.x*=-.45f;
        float z=scene.ballPosition.z+scene.ballVelocity.z*step;
        if (freeBall(scene.ballPosition.x,z)) scene.ballPosition.z=z;
        else scene.ballVelocity.z*=-.45f;
        Vec3 traveled=scene.ballPosition-before;
        const float length=std::sqrt(dot(traveled,traveled));
        if (length>1e-7f) {
            scene.ballOrientation=roll(traveled)*scene.ballOrientation;
            scene.ballRotation+=length/ballRadius;
        }
        scene.ballVelocity=scene.ballVelocity*std::exp(-1.45f*step);
        if (dot(scene.ballVelocity,scene.ballVelocity)<.0004f) scene.ballVelocity={0,0,0};
    }
}

Lighting lightingFor(const Scene& scene) {
    Lighting l;
    const float opening=std::clamp(scene.windowAngle/windowOpenAngle,0.f,1.f);
    const float transmission=.52f+.48f*opening;
    const float ambientTransmission=.82f+.18f*opening;
    if (scene.period==DayPeriod::Morning) {
        l.sky={.48f,.73f,.94f}; l.ambient=Vec3{.58f,.60f,.61f}*ambientTransmission;
        l.natural=Vec3{1.05f,1.02f,.94f}*transmission;
    } else if (scene.period==DayPeriod::Afternoon) {
        l.sky={.65f,.39f,.23f}; l.ambient=Vec3{.42f,.32f,.25f}*ambientTransmission;
        l.natural=Vec3{.78f,.44f,.22f}*transmission;
    } else {
        l.sky={.012f,.023f,.055f}; l.ambient=Vec3{.045f,.060f,.095f}*ambientTransmission;
        l.natural=Vec3{.035f,.055f,.10f}*transmission;
    }
    l.artificial=scene.lampOn ? Vec3{1.12f,.99f,.76f} : Vec3{0,0,0};
    return l;
}

std::string interactionPrompt(const Scene& scene, Interaction target) {
    if (target==Interaction::Door) {
        if (scene.doorBlocked) return "E - INVERTER PORTA / AFASTE-SE";
        return scene.doorOpen?"E - FECHAR PORTA":"E - ABRIR PORTA";
    }
    if (target==Interaction::Window) {
        if (scene.windowBlocked) return "E - INVERTER JANELA / AFASTE-SE";
        return scene.windowOpen?"E - FECHAR JANELA":"E - ABRIR JANELA";
    }
    if (target==Interaction::Lamp) return scene.lampOn?"E - DESLIGAR LUZ":"E - LIGAR LUZ";
    return "";
}
std::string statusText(const Scene& scene) {
    const char* periods[]={"MANHA","TARDE","NOITE"};
    return std::string("1/2/3 ")+periods[static_cast<int>(scene.period)]+" | LUZ "+
           (scene.lampOn?"LIGADA":"DESLIGADA");
}
