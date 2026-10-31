#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/wglext.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "scene.hpp"
#include "image_loader.hpp"
#include "celestial.hpp"
#include "lamp.hpp"
#include "behavior.hpp"
#include "hud.hpp"
#include "verification.hpp"
#include <fstream>

namespace {

// As funcoes modernas nao sao exportadas diretamente por opengl32.dll no Windows.
PFNGLCREATESHADERPROC glCreateShaderPtr;
PFNGLSHADERSOURCEPROC glShaderSourcePtr;
PFNGLCOMPILESHADERPROC glCompileShaderPtr;
PFNGLGETSHADERIVPROC glGetShaderivPtr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLogPtr;
PFNGLCREATEPROGRAMPROC glCreateProgramPtr;
PFNGLATTACHSHADERPROC glAttachShaderPtr;
PFNGLLINKPROGRAMPROC glLinkProgramPtr;
PFNGLGETPROGRAMIVPROC glGetProgramivPtr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLogPtr;
PFNGLDELETESHADERPROC glDeleteShaderPtr;
PFNGLDELETEPROGRAMPROC glDeleteProgramPtr;
PFNGLUSEPROGRAMPROC glUseProgramPtr;
PFNGLGENVERTEXARRAYSPROC glGenVertexArraysPtr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArrayPtr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArraysPtr;
PFNGLGENBUFFERSPROC glGenBuffersPtr;
PFNGLBINDBUFFERPROC glBindBufferPtr;
PFNGLBUFFERDATAPROC glBufferDataPtr;
PFNGLDELETEBUFFERSPROC glDeleteBuffersPtr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArrayPtr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointerPtr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocationPtr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fvPtr;
PFNGLUNIFORM3FPROC glUniform3fPtr;
PFNGLUNIFORM4FPROC glUniform4fPtr;
PFNGLUNIFORM1FPROC glUniform1fPtr;
PFNGLUNIFORM1IPROC glUniform1iPtr;
PFNGLGENERATEMIPMAPPROC glGenerateMipmapPtr;
PFNGLACTIVETEXTUREPROC glActiveTexturePtr;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffersPtr;
PFNGLBINDFRAMEBUFFERPROC glBindFramebufferPtr;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2DPtr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatusPtr;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffersPtr;

void* glFunction(const char* name) {
    void* p = reinterpret_cast<void*>(wglGetProcAddress(name));
    if (p == nullptr || p == reinterpret_cast<void*>(1) ||
        p == reinterpret_cast<void*>(2) || p == reinterpret_cast<void*>(3) ||
        p == reinterpret_cast<void*>(-1)) {
        static HMODULE library = LoadLibraryA("opengl32.dll");
        p = reinterpret_cast<void*>(GetProcAddress(library, name));
    }
    return p;
}

#define LOAD_GL(member, name) do { \
    member = reinterpret_cast<decltype(member)>(glFunction(name)); \
    if (!member) throw std::runtime_error(std::string("Funcao OpenGL ausente: ") + name); \
} while (false)

void loadOpenGL() {
    LOAD_GL(glCreateShaderPtr, "glCreateShader");
    LOAD_GL(glShaderSourcePtr, "glShaderSource");
    LOAD_GL(glCompileShaderPtr, "glCompileShader");
    LOAD_GL(glGetShaderivPtr, "glGetShaderiv");
    LOAD_GL(glGetShaderInfoLogPtr, "glGetShaderInfoLog");
    LOAD_GL(glCreateProgramPtr, "glCreateProgram");
    LOAD_GL(glAttachShaderPtr, "glAttachShader");
    LOAD_GL(glLinkProgramPtr, "glLinkProgram");
    LOAD_GL(glGetProgramivPtr, "glGetProgramiv");
    LOAD_GL(glGetProgramInfoLogPtr, "glGetProgramInfoLog");
    LOAD_GL(glDeleteShaderPtr, "glDeleteShader");
    LOAD_GL(glDeleteProgramPtr, "glDeleteProgram");
    LOAD_GL(glUseProgramPtr, "glUseProgram");
    LOAD_GL(glGenVertexArraysPtr, "glGenVertexArrays");
    LOAD_GL(glBindVertexArrayPtr, "glBindVertexArray");
    LOAD_GL(glDeleteVertexArraysPtr, "glDeleteVertexArrays");
    LOAD_GL(glGenBuffersPtr, "glGenBuffers");
    LOAD_GL(glBindBufferPtr, "glBindBuffer");
    LOAD_GL(glBufferDataPtr, "glBufferData");
    LOAD_GL(glDeleteBuffersPtr, "glDeleteBuffers");
    LOAD_GL(glEnableVertexAttribArrayPtr, "glEnableVertexAttribArray");
    LOAD_GL(glVertexAttribPointerPtr, "glVertexAttribPointer");
    LOAD_GL(glGetUniformLocationPtr, "glGetUniformLocation");
    LOAD_GL(glUniformMatrix4fvPtr, "glUniformMatrix4fv");
    LOAD_GL(glUniform3fPtr, "glUniform3f");
    LOAD_GL(glUniform4fPtr, "glUniform4f");
    LOAD_GL(glUniform1fPtr, "glUniform1f");
    LOAD_GL(glUniform1iPtr, "glUniform1i");
    LOAD_GL(glGenerateMipmapPtr, "glGenerateMipmap");
    LOAD_GL(glActiveTexturePtr, "glActiveTexture");
    LOAD_GL(glGenFramebuffersPtr, "glGenFramebuffers");
    LOAD_GL(glBindFramebufferPtr, "glBindFramebuffer");
    LOAD_GL(glFramebufferTexture2DPtr, "glFramebufferTexture2D");
    LOAD_GL(glCheckFramebufferStatusPtr, "glCheckFramebufferStatus");
    LOAD_GL(glDeleteFramebuffersPtr, "glDeleteFramebuffers");
}

GLuint compileShader(GLenum type, const char* source) {
    const GLuint shader = glCreateShaderPtr(type);
    glShaderSourcePtr(shader, 1, &source, nullptr);
    glCompileShaderPtr(shader);
    GLint ok = GL_FALSE;
    glGetShaderivPtr(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        std::array<char, 1024> log{};
        glGetShaderInfoLogPtr(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());
        glDeleteShaderPtr(shader);
        throw std::runtime_error(std::string("Erro no shader: ") + log.data());
    }
    return shader;
}

GLuint linkProgram(const char* vertexSource, const char* fragmentSource) {
    const GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSource);
    const GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    const GLuint program = glCreateProgramPtr();
    glAttachShaderPtr(program, vs);
    glAttachShaderPtr(program, fs);
    glLinkProgramPtr(program);
    glDeleteShaderPtr(vs);
    glDeleteShaderPtr(fs);
    GLint ok = GL_FALSE;
    glGetProgramivPtr(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        std::array<char, 1024> log{};
        glGetProgramInfoLogPtr(program, static_cast<GLsizei>(log.size()), nullptr, log.data());
        glDeleteProgramPtr(program);
        throw std::runtime_error(std::string("Erro ao ligar shaders: ") + log.data());
    }
    return program;
}

GLuint makeDepthProgram() {
    return linkProgram(R"GLSL(#version 330 core
layout(location=0) in vec3 aPosition;
uniform mat4 uLightMatrix;
uniform mat4 uModel;
void main() { gl_Position = uLightMatrix * uModel * vec4(aPosition,1.0); }
)GLSL", R"GLSL(#version 330 core
void main() {}
)GLSL");
}

GLuint makeProgram() {
    constexpr const char* vertexSource = R"GLSL(#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aUV;
uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;
uniform mat4 uNaturalShadowMatrix;
uniform mat4 uLampShadowMatrix;
out vec3 vNormal;
out vec3 vColor;
out vec3 vPosition;
out vec2 vUV;
out vec4 vNaturalShadow;
out vec4 vLampShadow;
void main() {
    vec4 world = uModel * vec4(aPosition, 1.0);
    vPosition = world.xyz;
    vNormal = transpose(inverse(mat3(uModel))) * aNormal;
    vColor = aColor;
    vUV = aUV;
    vNaturalShadow = uNaturalShadowMatrix * world;
    vLampShadow = uLampShadowMatrix * world;
    gl_Position = uProjection * uView * world;
}
)GLSL";
    constexpr const char* fragmentSource = R"GLSL(#version 330 core
in vec3 vNormal;
in vec3 vColor;
in vec3 vPosition;
in vec2 vUV;
in vec4 vNaturalShadow;
in vec4 vLampShadow;
uniform sampler2D uTexture;
uniform sampler2D uNaturalShadowMap;
uniform sampler2D uLampShadowMap;
uniform vec4 uColor;
uniform vec3 uMaterial; // intensidade especular, expoente, emissao
uniform vec3 uCamera;
uniform vec3 uAmbient;
uniform vec3 uNatural;
uniform vec3 uNaturalPosition;
uniform vec4 uSunOpening; // altura minima/maxima, z minimo/maximo
uniform float uSunBoundaryX;
uniform vec3 uArtificial;
uniform vec3 uLampPosition;
uniform vec3 uLampDirection;
uniform vec3 uLampCone; // cossenos externo e interno
uniform vec3 uOutdoorLight;
uniform int uOutdoor;
uniform int uGlass;
uniform vec3 uShadowActive; // natural, luminaria
out vec4 fragColor;
float shadowVisibility(sampler2D depthMap, vec4 lightSpace, vec3 normal,
                       vec3 direction, float enabled) {
    if (enabled < 0.5) return 1.0;
    if (lightSpace.w <= 0.0) return 0.0;
    vec3 p = lightSpace.xyz / lightSpace.w * 0.5 + 0.5;
    if (p.z <= 0.0 || p.z >= 1.0 || p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0)
        return 0.0;
    float bias = max(0.002 * (1.0 - max(dot(normal,direction),0.0)), 0.00065);
    vec2 texel = 1.0 / vec2(textureSize(depthMap,0));
    float lit = 0.0;
    for (int y=-1; y<=1; ++y)
        for (int x=-1; x<=1; ++x)
            lit += p.z - bias <= texture(depthMap,p.xy+vec2(x,y)*texel).r ? 1.0 : 0.0;
    // Um unico texel livre na borda de duas paredes nao deve virar um filete de luz.
    return smoothstep(0.15,0.85,lit / 9.0);
}
float entranceVisibility(vec3 point) {
    float span=point.x-uNaturalPosition.x;
    if (abs(span)<0.0001) return 0.0;
    float t=(uSunBoundaryX-uNaturalPosition.x)/span;
    if (t<=0.0 || t>1.001) return 0.0;
    vec3 hit=mix(uNaturalPosition,point,t);
    float clearance=min(min(hit.y-uSunOpening.x,uSunOpening.y-hit.y),
                        min(hit.z-uSunOpening.z,uSunOpening.w-hit.z));
    return smoothstep(0.0,0.02,clearance);
}
void main() {
    vec3 normal = normalize(vNormal);
    vec3 toLight = uLampPosition - vPosition;
    vec3 lightDir = normalize(toLight);
    vec3 viewDir = normalize(uCamera - vPosition);
    float attenuation = 1.0 / (1.0 + 0.035 * dot(toLight, toLight));
    float coneCos = dot(normalize(vPosition - uLampPosition), uLampDirection);
    float cone = smoothstep(uLampCone.x, uLampCone.y, coneCos);
    vec3 artificial = uArtificial * cone;
    float diffuse = max(dot(normal, lightDir), 0.0);
    float specular = diffuse > 0.0 ? pow(max(dot(normal, normalize(lightDir + viewDir)), 0.0), uMaterial.y) : 0.0;
    vec3 toWindow = uNaturalPosition - vPosition;
    vec3 sunDir = normalize(toWindow);
    float sunAttenuation = 1.0 / (1.0 + 0.018 * dot(toWindow, toWindow));
    float daylight = max(dot(normal, sunDir), 0.0);
    float sunSpecular = daylight > 0.0 ? pow(max(dot(normal, normalize(sunDir + viewDir)), 0.0), uMaterial.y) : 0.0;
    vec3 albedo = texture(uTexture, vUV).rgb * vColor * uColor.rgb;
    float sunShadow = shadowVisibility(uNaturalShadowMap,vNaturalShadow,normal,
                                       sunDir,uShadowActive.x) * entranceVisibility(vPosition);
    float lampShadow = shadowVisibility(uLampShadowMap,vLampShadow,normal,
                                        lightDir,uShadowActive.y);
    // Uma base ambiente discreta deixa as faces voltadas para a luz mais claras.
    vec3 direct = uNatural * daylight * sunAttenuation * sunShadow
                + artificial * (0.05 + 0.95 * diffuse) * attenuation * lampShadow;
    vec3 lit = albedo * (0.62 * uAmbient + direct);
    vec3 highlight = uMaterial.x * (1.7 * artificial * specular * attenuation * lampShadow
                                   + 0.65 * uNatural * sunSpecular * sunAttenuation * sunShadow);
    lit += highlight;
    float opacity = uColor.a;
    if (uGlass != 0) {
        // Schlick simplificado: transparencia frontal e bordas mais refletivas.
        float facing = abs(dot(normal, viewDir));
        float fresnel = pow(1.0 - clamp(facing, 0.0, 1.0), 3.0);
        float glint = length(artificial) * specular * attenuation * lampShadow
                    + length(uNatural) * sunSpecular * sunAttenuation * sunShadow;
        float sheen = clamp(0.12 + 0.70 * fresnel + 0.12 * glint, 0.0, 0.9);
        vec3 reflection = vec3(0.78, 0.89, 1.0)
                        * clamp(length(uAmbient + uNatural * sunShadow
                                       + artificial * lampShadow), 0.15, 1.0);
        lit = mix(0.42 * albedo * (uAmbient + direct), reflection, sheen);
        opacity = clamp(uColor.a + 0.43 * fresnel + 0.12 * glint,
                        uColor.a, 0.68);
    }
    if (uOutdoor != 0) {
        float skyFacing = max(dot(normal, normalize(vec3(0.35, 0.85, 0.4))), 0.0);
        lit = albedo * uOutdoorLight * (0.72 + 0.28 * skyFacing);
    }
    lit = mix(lit, albedo, uMaterial.z);
    fragColor = vec4(lit, opacity);
}
)GLSL";
    return linkProgram(vertexSource,fragmentSource);
}

struct GpuMesh { GLuint vao=0, vbo=0; GLsizei count=0; };
struct ShadowMap { GLuint texture=0, framebuffer=0; };
constexpr int shadowSize=1024;

struct Renderer {
    GLuint program=0, depthProgram=0;
    ShadowMap sunMap, lampMap;
    std::vector<GpuMesh> meshes;
    std::vector<GLuint> textures;
    std::vector<Material> materials=makeMaterials();
    GLint projection, view, modelMatrix, color, material, eye, ambient, natural, artificial;
    GLint outdoorLight, outdoor, glass;
    GLint naturalPositionUniform, sunOpening, sunBoundaryX;
    GLint naturalShadowMatrix, lampShadowMatrix, shadowActive;
    GLint depthLightMatrix, depthModelMatrix;
    GpuMesh hud;

    Renderer() {
        program=makeProgram();
        depthProgram=makeDepthProgram();
        glUseProgramPtr(program);
        projection=glGetUniformLocationPtr(program,"uProjection");
        view=glGetUniformLocationPtr(program,"uView");
        modelMatrix=glGetUniformLocationPtr(program,"uModel");
        color=glGetUniformLocationPtr(program,"uColor");
        material=glGetUniformLocationPtr(program,"uMaterial");
        eye=glGetUniformLocationPtr(program,"uCamera");
        ambient=glGetUniformLocationPtr(program,"uAmbient");
        natural=glGetUniformLocationPtr(program,"uNatural");
        naturalPositionUniform=glGetUniformLocationPtr(program,"uNaturalPosition");
        sunOpening=glGetUniformLocationPtr(program,"uSunOpening");
        sunBoundaryX=glGetUniformLocationPtr(program,"uSunBoundaryX");
        artificial=glGetUniformLocationPtr(program,"uArtificial");
        naturalShadowMatrix=glGetUniformLocationPtr(program,"uNaturalShadowMatrix");
        lampShadowMatrix=glGetUniformLocationPtr(program,"uLampShadowMatrix");
        shadowActive=glGetUniformLocationPtr(program,"uShadowActive");
        outdoorLight=glGetUniformLocationPtr(program,"uOutdoorLight");
        outdoor=glGetUniformLocationPtr(program,"uOutdoor");
        glass=glGetUniformLocationPtr(program,"uGlass");
        glUniform3fPtr(glGetUniformLocationPtr(program,"uLampPosition"),
                       lampEmitter.x,lampEmitter.y,lampEmitter.z);
        glUniform3fPtr(glGetUniformLocationPtr(program,"uLampDirection"),
                       lampDirection.x,lampDirection.y,lampDirection.z);
        glUniform3fPtr(glGetUniformLocationPtr(program,"uLampCone"),
                       lampConeOuterCos,lampConeInnerCos,0.f);
        glUniform1iPtr(glGetUniformLocationPtr(program,"uTexture"),0);
        glUniform1iPtr(glGetUniformLocationPtr(program,"uNaturalShadowMap"),1);
        glUniform1iPtr(glGetUniformLocationPtr(program,"uLampShadowMap"),2);
        glUseProgramPtr(depthProgram);
        depthLightMatrix=glGetUniformLocationPtr(depthProgram,"uLightMatrix");
        depthModelMatrix=glGetUniformLocationPtr(depthProgram,"uModel");
        glUseProgramPtr(program);
        for (int i=0;i<int(MeshKind::Count);++i) {
            auto vertices=makeMesh(static_cast<MeshKind>(i));
            GpuMesh mesh;
            mesh.count=static_cast<GLsizei>(vertices.size());
            glGenVertexArraysPtr(1,&mesh.vao);
            glGenBuffersPtr(1,&mesh.vbo);
            glBindVertexArrayPtr(mesh.vao);
            glBindBufferPtr(GL_ARRAY_BUFFER,mesh.vbo);
            glBufferDataPtr(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(Vertex)),vertices.data(),GL_STATIC_DRAW);
            const std::size_t offsets[]={offsetof(Vertex,position),offsetof(Vertex,normal),offsetof(Vertex,color),offsetof(Vertex,uv)};
            for (GLuint a=0;a<4;++a) {
                glEnableVertexAttribArrayPtr(a);
                glVertexAttribPointerPtr(a,a==3?2:3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsets[a]));
            }
            meshes.push_back(mesh);
        }
        glGenVertexArraysPtr(1,&hud.vao);
        glGenBuffersPtr(1,&hud.vbo);
        glBindVertexArrayPtr(hud.vao);
        glBindBufferPtr(GL_ARRAY_BUFFER,hud.vbo);
        const std::size_t offsets[]={offsetof(Vertex,position),offsetof(Vertex,normal),offsetof(Vertex,color),offsetof(Vertex,uv)};
        for (GLuint a=0;a<4;++a) {
            glEnableVertexAttribArrayPtr(a);
            glVertexAttribPointerPtr(a,a==3?2:3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsets[a]));
        }
        for (int i=0;i<int(TextureKind::Count);++i) {
            GLuint texture;
            glGenTextures(1,&texture);
            glBindTexture(GL_TEXTURE_2D,texture);
            constexpr int size=256;
            const TextureKind kind=static_cast<TextureKind>(i);
            ImageData image;
            if (kind==TextureKind::Art) image=loadArtworkImage();
            else image={size,size,makeTexture(kind,size)};
            glPixelStorei(GL_UNPACK_ALIGNMENT,1);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,image.width,image.height,0,
                         GL_RGB,GL_UNSIGNED_BYTE,image.pixels.data());
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glGenerateMipmapPtr(GL_TEXTURE_2D);
            textures.push_back(texture);
        }
        auto makeShadowMap=[&](ShadowMap& map) {
            glGenTextures(1,&map.texture);
            glBindTexture(GL_TEXTURE_2D,map.texture);
            glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,shadowSize,shadowSize,
                         0,GL_DEPTH_COMPONENT,GL_UNSIGNED_INT,nullptr);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
            const GLfloat darkBorder[]={0,0,0,0};
            glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,darkBorder);
            glGenFramebuffersPtr(1,&map.framebuffer);
            glBindFramebufferPtr(GL_FRAMEBUFFER,map.framebuffer);
            glFramebufferTexture2DPtr(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,map.texture,0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            if (glCheckFramebufferStatusPtr(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
                throw std::runtime_error("Framebuffer do mapa de sombras incompleto.");
        };
        makeShadowMap(sunMap);
        makeShadowMap(lampMap);
        glBindFramebufferPtr(GL_FRAMEBUFFER,0);
        glDrawBuffer(GL_BACK);
        glReadBuffer(GL_BACK);
        glBindTexture(GL_TEXTURE_2D,0);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }

    void drawPart(const Scene& scene, const Part& part) {
        const Mat4 transform=partModel(scene,part);
        auto m=materials[static_cast<int>(part.material)];
        if (part.object=="luminaria" && part.material==MaterialId::Light && !scene.lampOn) {
            m.emission=0;
            m.color={.62f,.61f,.55f};
        }
        if (part.object=="sol" && scene.period==DayPeriod::Afternoon)
            m.color={1.f,.53f,.28f};
        const auto& mesh=meshes[static_cast<int>(part.mesh)];
        glUniformMatrix4fvPtr(modelMatrix,1,GL_FALSE,transform.values.data());
        glUniform4fPtr(color,m.color.x,m.color.y,m.color.z,m.alpha);
        glUniform3fPtr(material,m.specular,m.shininess,m.emission);
        glUniform1iPtr(outdoor,part.outdoor ? 1 : 0);
        glUniform1iPtr(glass,part.material==MaterialId::Glass ? 1 : 0);
        glBindTexture(GL_TEXTURE_2D,textures[static_cast<int>(m.texture)]);
        glBindVertexArrayPtr(mesh.vao);
        glDrawArrays(GL_TRIANGLES,0,mesh.count);
    }

    void renderShadow(const Scene& scene, const ShadowMap& map, const Mat4& lightMatrix) {
        glBindFramebufferPtr(GL_FRAMEBUFFER,map.framebuffer);
        glViewport(0,0,shadowSize,shadowSize);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgramPtr(depthProgram);
        glUniformMatrix4fvPtr(depthLightMatrix,1,GL_FALSE,lightMatrix.values.data());
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.5f,2.f);
        for (const auto& part : scene.parts) {
            const bool shell=part.mesh==MeshKind::Shell;
            const bool furniture=part.object=="cama" || part.object=="mesa" ||
                                 part.object=="cadeira" || part.object=="guarda-roupas";
            const bool moving=part.group==Group::Door || part.group==Group::Window ||
                              part.group==Group::Ball;
            if (part.outdoor || !visiblePart(scene,part) ||
                (!shell && !furniture && !moving) ||
                materials[static_cast<int>(part.material)].alpha<.99f) continue;
            const Mat4 transform=partModel(scene,part);
            glUniformMatrix4fvPtr(depthModelMatrix,1,GL_FALSE,transform.values.data());
            const auto& mesh=meshes[static_cast<int>(part.mesh)];
            glBindVertexArrayPtr(mesh.vao);
            glDrawArrays(GL_TRIANGLES,0,mesh.count);
        }
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    void draw(const Scene& scene, Vec3 camera, const Mat4& projectionMatrix,
              const Mat4& viewMatrix, int width, int height) {
        const Vec3 sun=naturalPosition(scene.period);
        const Mat4 sunMatrix=perspective(120.f*pi/180.f,1.f,.2f,25.f)*
                             lookAt(sun,{4.25f,1.f,2.375f});
        const Mat4 lampMatrix=perspective(2.f*std::acos(lampConeOuterCos)+.04f,
                                          1.f,.04f,9.f)*
                              lookAt(lampEmitter,lampEmitter+lampDirection);
        renderShadow(scene,sunMap,sunMatrix);
        if (scene.lampOn) renderShadow(scene,lampMap,lampMatrix);
        glBindFramebufferPtr(GL_FRAMEBUFFER,0);
        glDrawBuffer(GL_BACK);
        glViewport(0,0,width,height);
        glActiveTexturePtr(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,sunMap.texture);
        glActiveTexturePtr(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D,lampMap.texture);
        glActiveTexturePtr(GL_TEXTURE0);
        glUseProgramPtr(program);
        glUniformMatrix4fvPtr(projection,1,GL_FALSE,projectionMatrix.values.data());
        glUniformMatrix4fvPtr(view,1,GL_FALSE,viewMatrix.values.data());
        glUniformMatrix4fvPtr(naturalShadowMatrix,1,GL_FALSE,sunMatrix.values.data());
        glUniformMatrix4fvPtr(lampShadowMatrix,1,GL_FALSE,lampMatrix.values.data());
        glUniform3fPtr(shadowActive,1.f,
                       scene.lampOn?1.f:0.f,0.f);
        glUniform3fPtr(naturalPositionUniform,sun.x,sun.y,sun.z);
        if (scene.period==DayPeriod::Afternoon) {
            glUniform4fPtr(sunOpening,0.f,doorTop,doorZMin,doorZMax);
            glUniform1fPtr(sunBoundaryX,roomWidth);
        } else {
            glUniform4fPtr(sunOpening,windowBottom,windowTop,windowZMin,windowZMax);
            glUniform1fPtr(sunBoundaryX,0.f);
        }
        glUniform3fPtr(eye,camera.x,camera.y,camera.z);
        const Lighting lights=lightingFor(scene);
        glUniform3fPtr(ambient,lights.ambient.x,lights.ambient.y,lights.ambient.z);
        glUniform3fPtr(natural,lights.natural.x,lights.natural.y,lights.natural.z);
        glUniform3fPtr(artificial,lights.artificial.x,lights.artificial.y,lights.artificial.z);
        Vec3 gardenLight{1.12f,1.12f,1.05f};
        if (scene.period==DayPeriod::Afternoon) gardenLight={1.05f,.72f,.49f};
        if (scene.period==DayPeriod::Night) gardenLight={.16f,.23f,.36f};
        glUniform3fPtr(outdoorLight,gardenLight.x,gardenLight.y,gardenLight.z);
        std::vector<const Part*> transparent;
        for (const auto& part : scene.parts) {
            if (!visiblePart(scene,part)) continue;
            if (materials[static_cast<int>(part.material)].alpha<1) transparent.push_back(&part);
            else drawPart(scene,part);
        }
        auto distance=[&](const Part* p) {
            Vec3 delta=transformPoint(partModel(scene,*p),{0,0,0})-camera;
            return dot(delta,delta);
        };
        std::sort(transparent.begin(),transparent.end(),[&](const Part* a,const Part* b){return distance(a)>distance(b);});
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        for (const Part* part : transparent) drawPart(scene,*part);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    void drawHud(int width, int height, const Scene& scene, Interaction target) {
        const auto vertices=makeHud(width,height,scene,target);
        const auto unit=identity();
        glDisable(GL_DEPTH_TEST);
        glUniformMatrix4fvPtr(projection,1,GL_FALSE,unit.values.data());
        glUniformMatrix4fvPtr(view,1,GL_FALSE,unit.values.data());
        glUniformMatrix4fvPtr(modelMatrix,1,GL_FALSE,unit.values.data());
        glUniform4fPtr(color,1,1,1,1);
        glUniform3fPtr(material,0,1,1);
        glUniform1iPtr(outdoor,0);
        glUniform1iPtr(glass,0);
        glBindTexture(GL_TEXTURE_2D,textures[static_cast<int>(TextureKind::White)]);
        glBindVertexArrayPtr(hud.vao);
        glBindBufferPtr(GL_ARRAY_BUFFER,hud.vbo);
        glBufferDataPtr(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(Vertex)),vertices.data(),GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES,0,static_cast<GLsizei>(vertices.size()));
        glEnable(GL_DEPTH_TEST);
    }

    void release() {
        glDeleteBuffersPtr(1,&hud.vbo);
        glDeleteVertexArraysPtr(1,&hud.vao);
        for (auto mesh : meshes) {
            glDeleteBuffersPtr(1,&mesh.vbo);
            glDeleteVertexArraysPtr(1,&mesh.vao);
        }
        glDeleteTextures(static_cast<GLsizei>(textures.size()),textures.data());
        glDeleteTextures(1,&sunMap.texture);
        glDeleteTextures(1,&lampMap.texture);
        glDeleteFramebuffersPtr(1,&sunMap.framebuffer);
        glDeleteFramebuffersPtr(1,&lampMap.framebuffer);
        glDeleteProgramPtr(depthProgram);
        glDeleteProgramPtr(program);
    }
};

// Capturas do framebuffer permitem conferir a cena sem depender do foco do mouse.
void saveFrame(const char* name, int width, int height) {
    const int stride=(width*3+3)&~3;
    std::vector<unsigned char> pixels(stride*height);
    glPixelStorei(GL_PACK_ALIGNMENT,4);
    glReadBuffer(GL_BACK);
    glReadPixels(0,0,width,height,GL_BGR,GL_UNSIGNED_BYTE,pixels.data());
    BITMAPFILEHEADER file{};
    BITMAPINFOHEADER info{};
    file.bfType=0x4D42;
    file.bfOffBits=sizeof(file)+sizeof(info);
    file.bfSize=file.bfOffBits+static_cast<DWORD>(pixels.size());
    info.biSize=sizeof(info); info.biWidth=width; info.biHeight=height;
    info.biPlanes=1; info.biBitCount=24; info.biCompression=BI_RGB;
    std::ofstream out(name,std::ios::binary);
    out.write(reinterpret_cast<const char*>(&file),sizeof(file));
    out.write(reinterpret_cast<const char*>(&info),sizeof(info));
    out.write(reinterpret_cast<const char*>(pixels.data()),static_cast<std::streamsize>(pixels.size()));
    if (!out) throw std::runtime_error(std::string("Falha ao salvar captura: ")+name);
}

HWND window = nullptr;
bool running = true;
bool focused = true;
bool cursorCaptured = false;
bool resetMouse = true;
int windowWidth = 1280;
int windowHeight = 720;
std::vector<char> commands;

void captureCursor(bool capture) {
    if (capture == cursorCaptured) return;
    ShowCursor(capture ? FALSE : TRUE);
    cursorCaptured = capture;
    if (capture) resetMouse = true;
}

LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SIZE:
        windowWidth = LOWORD(lParam);
        windowHeight = HIWORD(lParam);
        resetMouse = true;
        return 0;
    case WM_MOVE:
        resetMouse = true;
        return 0;
    case WM_SETFOCUS:
        focused = true;
        captureCursor(true);
        return 0;
    case WM_KILLFOCUS:
        focused = false;
        commands.clear();
        captureCursor(false);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) running = false;
        if (!(lParam & (1LL<<30)) && (wParam=='E' || (wParam>='1' && wParam<='3')))
            commands.push_back(static_cast<char>(wParam));
        return 0;
    case WM_CLOSE:
        running = false;
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

void centerCursor() {
    RECT client{};
    GetClientRect(window, &client);
    POINT center{(client.right - client.left) / 2, (client.bottom - client.top) / 2};
    ClientToScreen(window, &center);
    SetCursorPos(center.x, center.y);
}

void verifyWindowEvents() {
    commands.clear();
    PostMessageW(window,WM_KEYDOWN,'E',0);
    PostMessageW(window,WM_KEYDOWN,'E',1LL<<30); // Autorrepeticao deve ser ignorada.
    for (char key : {'2','3','1'}) PostMessageW(window,WM_KEYDOWN,key,0);
    MSG message{};
    while (PeekMessageW(&message,nullptr,0,0,PM_REMOVE)) DispatchMessageW(&message);
    if (std::string(commands.begin(),commands.end())!="E231")
        throw std::runtime_error("Falha no despacho de teclas/autorrepeticao Win32.");
    Scene probe=makeScene();
    Vec3 camera{7,1.65f,4.2f}, target{8.46f,1.1f,4.075f};
    for (char key : commands) applyCommand(probe,key,camera,normalized(target-camera));
    if (!probe.doorOpen || probe.period!=DayPeriod::Morning)
        throw std::runtime_error("Eventos Win32 nao chegaram as acoes.");
    commands.clear();
    RECT before{},client{};
    GetWindowRect(window,&before);
    MoveWindow(window,before.left,before.top,820,580,FALSE);
    GetClientRect(window,&client);
    if (windowWidth!=client.right || windowHeight!=client.bottom || !resetMouse)
        throw std::runtime_error("Redimensionamento nao atualizou o viewport.");
    MoveWindow(window,before.left,before.top,before.right-before.left,before.bottom-before.top,FALSE);
    commands.push_back('E');
    SendMessageW(window,WM_KILLFOCUS,0,0);
    if (!commands.empty() || cursorCaptured)
        throw std::runtime_error("Perda de foco nao limpou entrada/cursor.");
    std::cout << "OK: eventos Win32 E/1/2/3, autorrepeticao, tamanho e perda de foco.\n";
}

int run(bool verify) {
    SetProcessDPIAware();
    if (verify) { windowWidth=960; windowHeight=640; }
    const HINSTANCE instance = GetModuleHandleW(nullptr);
    WNDCLASSW klass{};
    klass.lpfnWndProc = windowProc;
    klass.hInstance = instance;
    klass.lpszClassName = L"QuartoOpenGL";
    klass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    if (!RegisterClassW(&klass)) throw std::runtime_error("Falha ao registrar a janela.");

    const DWORD style = WS_OVERLAPPEDWINDOW;
    RECT bounds{0, 0, windowWidth, windowHeight};
    AdjustWindowRect(&bounds, style, FALSE);
    window = CreateWindowExW(0, klass.lpszClassName, L"Quarto Interativo",
                             style, CW_USEDEFAULT, CW_USEDEFAULT,
                             bounds.right - bounds.left, bounds.bottom - bounds.top,
                             nullptr, nullptr, instance, nullptr);
    if (!window) throw std::runtime_error("Falha ao criar a janela.");
    HDC dc = GetDC(window);
    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;
    const int pixelFormat = ChoosePixelFormat(dc, &pfd);
    if (!pixelFormat || !SetPixelFormat(dc, pixelFormat, &pfd))
        throw std::runtime_error("Falha ao configurar o formato de pixels.");

    HGLRC temporary = wglCreateContext(dc);
    if (!temporary || !wglMakeCurrent(dc, temporary))
        throw std::runtime_error("Falha ao criar o contexto OpenGL temporario.");
    auto createContext = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(
        glFunction("wglCreateContextAttribsARB"));
    if (!createContext)
        throw std::runtime_error("OpenGL 3.3 nao disponivel neste computador.");
    const int attributes[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                              WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                              WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                              0};
    HGLRC context = createContext(dc, nullptr, attributes);
    if (!context) throw std::runtime_error("Falha ao criar contexto OpenGL 3.3.");
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(temporary);
    if (!wglMakeCurrent(dc, context))
        throw std::runtime_error("Falha ao ativar contexto OpenGL 3.3.");
    loadOpenGL();
    auto swapInterval = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(
        glFunction("wglSwapIntervalEXT"));
    if (swapInterval) swapInterval(1);

    Scene scene=makeScene();
    Renderer renderer;
    glClearColor(0.43f, 0.67f, 0.87f, 1.0f);
    std::size_t verificationFrame=0;
    std::vector<VerificationFrame> verificationFrames;
    if (verify) {
        verifyWindowEvents();
        verificationFrames=makeVerificationFrames();
    }

    Vec3 camera=initialCamera;
    const Vec3 initialDirection=normalized(initialTarget-camera);
    float yaw=std::atan2(initialDirection.z,initialDirection.x);
    float pitch=std::asin(initialDirection.y);
    if (!verify) {
        ShowWindow(window, SW_SHOW);
        UpdateWindow(window);
        captureCursor(true);
        centerCursor();
    }
    auto previous = std::chrono::steady_clock::now();
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';
    std::cout << "W/A/S/D: mover | mouse: olhar | E: interagir | 1/2/3: horario | Esc: sair\n";

    while (running) {
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) running = false;
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        if (!running) break;
        const auto now = std::chrono::steady_clock::now();
        const float dt = std::min(std::chrono::duration<float>(now - previous).count(), 0.05f);
        previous = now;

        if (!verify && focused && cursorCaptured) {
            if (resetMouse) {
                centerCursor();
                resetMouse = false;
            } else {
                RECT client{};
                GetClientRect(window, &client);
                POINT center{(client.right - client.left) / 2,
                             (client.bottom - client.top) / 2};
                ClientToScreen(window, &center);
                POINT mouse{};
                GetCursorPos(&mouse);
                yaw += static_cast<float>(mouse.x - center.x) * 0.0025f;
                pitch = std::clamp(pitch - static_cast<float>(mouse.y - center.y) * 0.0025f,
                                   -1.48f, 1.48f);
                SetCursorPos(center.x, center.y);
            }

            const Vec3 forward{std::cos(yaw), 0, std::sin(yaw)};
            const Vec3 right{-forward.z, 0, forward.x};
            Vec3 direction{0, 0, 0};
            auto held = [](int key) { return (GetAsyncKeyState(key) & 0x8000) != 0; };
            if (held('W')) direction = direction + forward;
            if (held('S')) direction = direction - forward;
            if (held('D')) direction = direction + right;
            if (held('A')) direction = direction - right;
            moveCamera(scene,camera,direction,dt);
            Vec3 lookDirection{std::cos(yaw)*std::cos(pitch),std::sin(pitch),std::sin(yaw)*std::cos(pitch)};
            for (char key : commands) applyCommand(scene,key,camera,lookDirection);
            updateScene(scene,dt,camera);
        }
        commands.clear();

        if (verify) {
            const auto& shot=verificationFrames[verificationFrame];
            scene=shot.scene;
            camera=shot.camera;
            Vec3 direction=normalized(shot.target-camera);
            yaw=std::atan2(direction.z,direction.x);
            pitch=std::asin(direction.y);
        }
        if (windowWidth > 0 && windowHeight > 0) {
            glViewport(0, 0, windowWidth, windowHeight);
            const Lighting lights=lightingFor(scene);
            glClearColor(lights.sky.x,lights.sky.y,lights.sky.z,1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            const Mat4 projection = perspective(70.0f * pi / 180.0f,
                                                float(windowWidth) / float(windowHeight),
                                                0.05f, 50.0f);
            const Vec3 lookDirection{std::cos(yaw) * std::cos(pitch), std::sin(pitch),
                                     std::sin(yaw) * std::cos(pitch)};
            const Mat4 view = lookAt(camera, camera + lookDirection);
            renderer.draw(scene,camera,projection,view,windowWidth,windowHeight);
            renderer.drawHud(windowWidth,windowHeight,scene,aimedInteraction(scene,camera,lookDirection));
            if (verify) {
                saveFrame(verificationFrames[verificationFrame].file.c_str(),windowWidth,windowHeight);
                if (glGetError()!=GL_NO_ERROR) throw std::runtime_error("Erro OpenGL durante verificacao.");
                std::cout << "Captura: " << verificationFrames[verificationFrame].file
                          << " | porta=" << scene.doorAngle << " janela=" << scene.windowAngle
                          << " bola=" << scene.ballPosition.x << "," << scene.ballPosition.z
                          << " giro=" << scene.ballRotation << " | " << statusText(scene) << std::endl;
                ++verificationFrame;
                if (verificationFrame==verificationFrames.size()) running=false;
            }
            SwapBuffers(dc);
        } else {
            Sleep(16);
        }
    }

    captureCursor(false);
    renderer.release();
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(context);
    ReleaseDC(window, dc);
    DestroyWindow(window);
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        return run(argc > 1 && std::string(argv[1]) == "--verificar");
    } catch (const std::exception& error) {
        captureCursor(false);
        std::cerr << error.what() << '\n';
        if (!(argc > 1 && std::string(argv[1]) == "--verificar"))
            MessageBoxA(nullptr, error.what(), "Quarto Interativo", MB_OK | MB_ICONERROR);
        return 1;
    }
}
