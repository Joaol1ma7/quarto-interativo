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

void* glFunction(const char* nome) {
    void* p = reinterpret_cast<void*>(wglGetProcAddress(nome));
    if (p == nullptr || p == reinterpret_cast<void*>(1) ||
        p == reinterpret_cast<void*>(2) || p == reinterpret_cast<void*>(3) ||
        p == reinterpret_cast<void*>(-1)) {
        static HMODULE biblioteca = LoadLibraryA("opengl32.dll");
        p = reinterpret_cast<void*>(GetProcAddress(biblioteca, nome));
    }
    return p;
}

#define LOAD_GL(membro, nome) do { \
    membro = reinterpret_cast<decltype(membro)>(glFunction(nome)); \
    if (!membro) throw std::runtime_error(std::string("Funcao OpenGL ausente: ") + nome); \
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

GLuint compileShader(GLenum tipoGl, const char* fonte) {
    const GLuint sombreador = glCreateShaderPtr(tipoGl);
    glShaderSourcePtr(sombreador, 1, &fonte, nullptr);
    glCompileShaderPtr(sombreador);
    GLint ok = GL_FALSE;
    glGetShaderivPtr(sombreador, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        std::array<char, 1024> registro{};
        glGetShaderInfoLogPtr(sombreador, static_cast<GLsizei>(registro.size()), nullptr, registro.data());
        glDeleteShaderPtr(sombreador);
        throw std::runtime_error(std::string("Erro no sombreador: ") + registro.data());
    }
    return sombreador;
}

GLuint linkProgram(const char* fonteVertice, const char* fonteFragmento) {
    const GLuint vs = compileShader(GL_VERTEX_SHADER, fonteVertice);
    const GLuint fs = compileShader(GL_FRAGMENT_SHADER, fonteFragmento);
    const GLuint programa = glCreateProgramPtr();
    glAttachShaderPtr(programa, vs);
    glAttachShaderPtr(programa, fs);
    glLinkProgramPtr(programa);
    glDeleteShaderPtr(vs);
    glDeleteShaderPtr(fs);
    GLint ok = GL_FALSE;
    glGetProgramivPtr(programa, GL_LINK_STATUS, &ok);
    if (!ok) {
        std::array<char, 1024> registro{};
        glGetProgramInfoLogPtr(programa, static_cast<GLsizei>(registro.size()), nullptr, registro.data());
        glDeleteProgramPtr(programa);
        throw std::runtime_error(std::string("Erro ao ligar shaders: ") + registro.data());
    }
    return programa;
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
    constexpr const char* fonteVertice = R"GLSL(#version 330 core
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
    vec4 mundo = uModel * vec4(aPosition, 1.0);
    vPosition = mundo.xyz;
    vNormal = transpose(inversa(mat3(uModel))) * aNormal;
    vColor = aColor;
    vUV = aUV;
    vNaturalShadow = uNaturalShadowMatrix * mundo;
    vLampShadow = uLampShadowMatrix * mundo;
    gl_Position = uProjection * uView * mundo;
}
)GLSL";
    constexpr const char* fonteFragmento = R"GLSL(#version 330 core
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
float shadowVisibility(sampler2D mapaProfundidade, vec4 lightSpace, vec3 normal,
                       vec3 direcao, float habilitada) {
    if (habilitada < 0.5) return 1.0;
    if (lightSpace.w <= 0.0) return 0.0;
    vec3 p = lightSpace.xyz / lightSpace.w * 0.5 + 0.5;
    if (p.z <= 0.0 || p.z >= 1.0 || p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0)
        return 0.0;
    float deslocamentoSombra = max(0.002 * (1.0 - max(dot(normal,direcao),0.0)), 0.00065);
    vec2 texel = 1.0 / vec2(tamanhoTextura(mapaProfundidade,0));
    float iluminacao = 0.0;
    for (int y=-1; y<=1; ++y)
        for (int x=-1; x<=1; ++x)
            iluminacao += p.z - deslocamentoSombra <= texture(mapaProfundidade,p.xy+vec2(x,y)*texel).r ? 1.0 : 0.0;
    // Um unico texel livre na borda de duas paredes nao deve virar um filete de luz.
    return smoothstep(0.15,0.85,iluminacao / 9.0);
}
float entranceVisibility(vec3 ponto) {
    float intervalo=ponto.x-uNaturalPosition.x;
    if (abs(intervalo)<0.0001) return 0.0;
    float t=(uSunBoundaryX-uNaturalPosition.x)/intervalo;
    if (t<=0.0 || t>1.001) return 0.0;
    vec3 impacto=mix(uNaturalPosition,ponto,t);
    float folga=min(min(impacto.y-uSunOpening.x,uSunOpening.y-impacto.y),
                        min(impacto.z-uSunOpening.z,uSunOpening.w-impacto.z));
    return smoothstep(0.0,0.02,folga);
}
void main() {
    vec3 normal = normalize(vNormal);
    vec3 toLight = uLampPosition - vPosition;
    vec3 direcaoLuz = normalize(toLight);
    vec3 direcaoVisao = normalize(uCamera - vPosition);
    float atenuacao = 1.0 / (1.0 + 0.035 * dot(toLight, toLight));
    float cossenoCone = dot(normalize(vPosition - uLampPosition), uLampDirection);
    float cone = smoothstep(uLampCone.x, uLampCone.y, cossenoCone);
    vec3 artificial = uArtificial * cone;
    float difusa = max(dot(normal, direcaoLuz), 0.0);
    float especular = difusa > 0.0 ? pow(max(dot(normal, normalize(direcaoLuz + direcaoVisao)), 0.0), uMaterial.y) : 0.0;
    vec3 toWindow = uNaturalPosition - vPosition;
    vec3 sunDir = normalize(toWindow);
    float atenuacaoSol = 1.0 / (1.0 + 0.018 * dot(toWindow, toWindow));
    float luzDia = max(dot(normal, sunDir), 0.0);
    float especularSol = luzDia > 0.0 ? pow(max(dot(normal, normalize(sunDir + direcaoVisao)), 0.0), uMaterial.y) : 0.0;
    vec3 albedo = texture(uTexture, vUV).rgb * vColor * uColor.rgb;
    float sombraSol = shadowVisibility(uNaturalShadowMap,vNaturalShadow,normal,
                                       sunDir,uShadowActive.x) * entranceVisibility(vPosition);
    float sombraLuminaria = shadowVisibility(uLampShadowMap,vLampShadow,normal,
                                        direcaoLuz,uShadowActive.y);
    // Uma base ambiente discreta deixa as faces voltadas para a luz mais claras.
    vec3 direto = uNatural * luzDia * atenuacaoSol * sombraSol
                + artificial * (0.05 + 0.95 * difusa) * atenuacao * sombraLuminaria;
    vec3 iluminacao = albedo * (0.62 * uAmbient + direto);
    vec3 highlight = uMaterial.x * (1.7 * artificial * especular * atenuacao * sombraLuminaria
                                   + 0.65 * uNatural * especularSol * atenuacaoSol * sombraSol);
    iluminacao += highlight;
    float opacidade = uColor.a;
    if (uGlass != 0) {
        // Schlick simplificado: transparencia frontal e bordas mais refletivas.
        float frontalidade = abs(dot(normal, direcaoVisao));
        float fresnel = pow(1.0 - clamp(frontalidade, 0.0, 1.0), 3.0);
        float reflexo = length(artificial) * especular * atenuacao * sombraLuminaria
                    + length(uNatural) * especularSol * atenuacaoSol * sombraSol;
        float lustro = clamp(0.12 + 0.70 * fresnel + 0.12 * reflexo, 0.0, 0.9);
        vec3 reflection = vec3(0.78, 0.89, 1.0)
                        * clamp(length(uAmbient + uNatural * sombraSol
                                       + artificial * sombraLuminaria), 0.15, 1.0);
        iluminacao = mix(0.42 * albedo * (uAmbient + direto), reflection, lustro);
        opacidade = clamp(uColor.a + 0.43 * fresnel + 0.12 * reflexo,
                        uColor.a, 0.68);
    }
    if (uOutdoor != 0) {
        float orientacaoCeu = max(dot(normal, normalize(vec3(0.35, 0.85, 0.4))), 0.0);
        iluminacao = albedo * uOutdoorLight * (0.72 + 0.28 * orientacaoCeu);
    }
    iluminacao = mix(iluminacao, albedo, uMaterial.z);
    fragColor = vec4(iluminacao, opacidade);
}
)GLSL";
    return linkProgram(fonteVertice,fonteFragmento);
}

struct GpuMesh { GLuint vao=0, vbo=0; GLsizei count=0; };
struct ShadowMap { GLuint texture=0, bufferQuadro=0; };
constexpr int tamanhoSombra=1024;

struct Renderer {
    GLuint programa=0, programaProfundidade=0;
    ShadowMap mapaSol, mapaLuminaria;
    std::vector<GpuMesh> malhas;
    std::vector<GLuint> texturas;
    std::vector<Material> materiais=makeMaterials();
    GLint projecao, visao, matrizModelo, cor, material, olho, ambiente, natural, artificial;
    GLint luzExterna, externo, vidro;
    GLint posicaoNaturalUniforme, aberturaSol, limiteSolX;
    GLint matrizSombraNatural, matrizSombraLuminaria, sombraAtiva;
    GLint matrizLuzProfundidade, matrizModeloProfundidade;
    GpuMesh painel;

    Renderer() {
        programa=makeProgram();
        programaProfundidade=makeDepthProgram();
        glUseProgramPtr(programa);
        projecao=glGetUniformLocationPtr(programa,"uProjection");
        visao=glGetUniformLocationPtr(programa,"uView");
        matrizModelo=glGetUniformLocationPtr(programa,"uModel");
        cor=glGetUniformLocationPtr(programa,"uColor");
        material=glGetUniformLocationPtr(programa,"uMaterial");
        olho=glGetUniformLocationPtr(programa,"uCamera");
        ambiente=glGetUniformLocationPtr(programa,"uAmbient");
        natural=glGetUniformLocationPtr(programa,"uNatural");
        posicaoNaturalUniforme=glGetUniformLocationPtr(programa,"uNaturalPosition");
        aberturaSol=glGetUniformLocationPtr(programa,"uSunOpening");
        limiteSolX=glGetUniformLocationPtr(programa,"uSunBoundaryX");
        artificial=glGetUniformLocationPtr(programa,"uArtificial");
        matrizSombraNatural=glGetUniformLocationPtr(programa,"uNaturalShadowMatrix");
        matrizSombraLuminaria=glGetUniformLocationPtr(programa,"uLampShadowMatrix");
        sombraAtiva=glGetUniformLocationPtr(programa,"uShadowActive");
        luzExterna=glGetUniformLocationPtr(programa,"uOutdoorLight");
        externo=glGetUniformLocationPtr(programa,"uOutdoor");
        vidro=glGetUniformLocationPtr(programa,"uGlass");
        glUniform3fPtr(glGetUniformLocationPtr(programa,"uLampPosition"),
                       lampEmitter.x,lampEmitter.y,lampEmitter.z);
        glUniform3fPtr(glGetUniformLocationPtr(programa,"uLampDirection"),
                       lampDirection.x,lampDirection.y,lampDirection.z);
        glUniform3fPtr(glGetUniformLocationPtr(programa,"uLampCone"),
                       lampConeOuterCos,lampConeInnerCos,0.f);
        glUniform1iPtr(glGetUniformLocationPtr(programa,"uTexture"),0);
        glUniform1iPtr(glGetUniformLocationPtr(programa,"uNaturalShadowMap"),1);
        glUniform1iPtr(glGetUniformLocationPtr(programa,"uLampShadowMap"),2);
        glUseProgramPtr(programaProfundidade);
        matrizLuzProfundidade=glGetUniformLocationPtr(programaProfundidade,"uLightMatrix");
        matrizModeloProfundidade=glGetUniformLocationPtr(programaProfundidade,"uModel");
        glUseProgramPtr(programa);
        for (int i=0;i<int(MeshKind::Count);++i) {
            auto vertices=makeMesh(static_cast<MeshKind>(i));
            GpuMesh malha;
            malha.count=static_cast<GLsizei>(vertices.size());
            glGenVertexArraysPtr(1,&malha.vao);
            glGenBuffersPtr(1,&malha.vbo);
            glBindVertexArrayPtr(malha.vao);
            glBindBufferPtr(GL_ARRAY_BUFFER,malha.vbo);
            glBufferDataPtr(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(Vertex)),vertices.data(),GL_STATIC_DRAW);
            const std::size_t deslocamentos[]={offsetof(Vertex,posicao),offsetof(Vertex,normal),offsetof(Vertex,cor),offsetof(Vertex,uv)};
            for (GLuint a=0;a<4;++a) {
                glEnableVertexAttribArrayPtr(a);
                glVertexAttribPointerPtr(a,a==3?2:3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(deslocamentos[a]));
            }
            malhas.push_back(malha);
        }
        glGenVertexArraysPtr(1,&painel.vao);
        glGenBuffersPtr(1,&painel.vbo);
        glBindVertexArrayPtr(painel.vao);
        glBindBufferPtr(GL_ARRAY_BUFFER,painel.vbo);
        const std::size_t deslocamentos[]={offsetof(Vertex,posicao),offsetof(Vertex,normal),offsetof(Vertex,cor),offsetof(Vertex,uv)};
        for (GLuint a=0;a<4;++a) {
            glEnableVertexAttribArrayPtr(a);
            glVertexAttribPointerPtr(a,a==3?2:3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(deslocamentos[a]));
        }
        for (int i=0;i<int(TextureKind::Count);++i) {
            GLuint texture;
            glGenTextures(1,&texture);
            glBindTexture(GL_TEXTURE_2D,texture);
            constexpr int size=256;
            const TextureKind tipo=static_cast<TextureKind>(i);
            ImageData imagem;
            if (tipo==TextureKind::Art) imagem=loadArtworkImage();
            else imagem={size,size,makeTexture(tipo,size)};
            glPixelStorei(GL_UNPACK_ALIGNMENT,1);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,imagem.largura,imagem.altura,0,
                         GL_RGB,GL_UNSIGNED_BYTE,imagem.pixels.data());
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glGenerateMipmapPtr(GL_TEXTURE_2D);
            texturas.push_back(texture);
        }
        auto makeShadowMap=[&](ShadowMap& mapa) {
            glGenTextures(1,&mapa.texture);
            glBindTexture(GL_TEXTURE_2D,mapa.texture);
            glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,tamanhoSombra,tamanhoSombra,
                         0,GL_DEPTH_COMPONENT,GL_UNSIGNED_INT,nullptr);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
            const GLfloat bordaEscura[]={0,0,0,0};
            glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,bordaEscura);
            glGenFramebuffersPtr(1,&mapa.bufferQuadro);
            glBindFramebufferPtr(GL_FRAMEBUFFER,mapa.bufferQuadro);
            glFramebufferTexture2DPtr(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,mapa.texture,0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            if (glCheckFramebufferStatusPtr(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
                throw std::runtime_error("Framebuffer do mapa de sombras incompleto.");
        };
        makeShadowMap(mapaSol);
        makeShadowMap(mapaLuminaria);
        glBindFramebufferPtr(GL_FRAMEBUFFER,0);
        glDrawBuffer(GL_BACK);
        glReadBuffer(GL_BACK);
        glBindTexture(GL_TEXTURE_2D,0);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }

    void drawPart(const Scene& cena, const Part& parte) {
        const Mat4 transformacao=partModel(cena,parte);
        auto m=materiais[static_cast<int>(parte.material)];
        if (parte.objeto=="luminaria" && parte.material==MaterialId::Light && !cena.luminariaLigada) {
            m.emissao=0;
            m.cor={.62f,.61f,.55f};
        }
        if (parte.objeto=="sol" && cena.periodo==DayPeriod::Afternoon)
            m.cor={1.f,.53f,.28f};
        const auto& malha=malhas[static_cast<int>(parte.malha)];
        glUniformMatrix4fvPtr(matrizModelo,1,GL_FALSE,transformacao.valores.data());
        glUniform4fPtr(cor,m.cor.x,m.cor.y,m.cor.z,m.alpha);
        glUniform3fPtr(material,m.especular,m.brilho,m.emissao);
        glUniform1iPtr(externo,parte.externo ? 1 : 0);
        glUniform1iPtr(vidro,parte.material==MaterialId::Glass ? 1 : 0);
        glBindTexture(GL_TEXTURE_2D,texturas[static_cast<int>(m.texture)]);
        glBindVertexArrayPtr(malha.vao);
        glDrawArrays(GL_TRIANGLES,0,malha.count);
    }

    void renderShadow(const Scene& cena, const ShadowMap& mapa, const Mat4& matrizLuz) {
        glBindFramebufferPtr(GL_FRAMEBUFFER,mapa.bufferQuadro);
        glViewport(0,0,tamanhoSombra,tamanhoSombra);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgramPtr(programaProfundidade);
        glUniformMatrix4fvPtr(matrizLuzProfundidade,1,GL_FALSE,matrizLuz.valores.data());
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.5f,2.f);
        for (const auto& parte : cena.partes) {
            const bool estrutura=parte.malha==MeshKind::Shell;
            const bool movel=parte.objeto=="cama" || parte.objeto=="mesa" ||
                                 parte.objeto=="cadeira" || parte.objeto=="guarda-roupas";
            const bool movendo=parte.grupo==Group::Door || parte.grupo==Group::Window ||
                              parte.grupo==Group::Ball;
            if (parte.externo || !visiblePart(cena,parte) ||
                (!estrutura && !movel && !movendo) ||
                materiais[static_cast<int>(parte.material)].alpha<.99f) continue;
            const Mat4 transformacao=partModel(cena,parte);
            glUniformMatrix4fvPtr(matrizModeloProfundidade,1,GL_FALSE,transformacao.valores.data());
            const auto& malha=malhas[static_cast<int>(parte.malha)];
            glBindVertexArrayPtr(malha.vao);
            glDrawArrays(GL_TRIANGLES,0,malha.count);
        }
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    void draw(const Scene& cena, Vec3 camera, const Mat4& matrizProjecao,
              const Mat4& matrizVisao, int largura, int altura) {
        const Vec3 sol=naturalPosition(cena.periodo);
        const Mat4 matrizSol=perspective(120.f*pi/180.f,1.f,.2f,25.f)*
                             lookAt(sol,{4.25f,1.f,2.375f});
        const Mat4 matrizLuminaria=perspective(2.f*std::acos(lampConeOuterCos)+.04f,
                                          1.f,.04f,9.f)*
                              lookAt(lampEmitter,lampEmitter+lampDirection);
        renderShadow(cena,mapaSol,matrizSol);
        if (cena.luminariaLigada) renderShadow(cena,mapaLuminaria,matrizLuminaria);
        glBindFramebufferPtr(GL_FRAMEBUFFER,0);
        glDrawBuffer(GL_BACK);
        glViewport(0,0,largura,altura);
        glActiveTexturePtr(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,mapaSol.texture);
        glActiveTexturePtr(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D,mapaLuminaria.texture);
        glActiveTexturePtr(GL_TEXTURE0);
        glUseProgramPtr(programa);
        glUniformMatrix4fvPtr(projecao,1,GL_FALSE,matrizProjecao.valores.data());
        glUniformMatrix4fvPtr(visao,1,GL_FALSE,matrizVisao.valores.data());
        glUniformMatrix4fvPtr(matrizSombraNatural,1,GL_FALSE,matrizSol.valores.data());
        glUniformMatrix4fvPtr(matrizSombraLuminaria,1,GL_FALSE,matrizLuminaria.valores.data());
        glUniform3fPtr(sombraAtiva,1.f,
                       cena.luminariaLigada?1.f:0.f,0.f);
        glUniform3fPtr(posicaoNaturalUniforme,sol.x,sol.y,sol.z);
        if (cena.periodo==DayPeriod::Afternoon) {
            glUniform4fPtr(aberturaSol,0.f,topoPorta,portaZMin,portaZMax);
            glUniform1fPtr(limiteSolX,larguraQuarto);
        } else {
            glUniform4fPtr(aberturaSol,baseJanela,topoJanela,janelaZMin,janelaZMax);
            glUniform1fPtr(limiteSolX,0.f);
        }
        glUniform3fPtr(olho,camera.x,camera.y,camera.z);
        const Lighting luzes=lightingFor(cena);
        glUniform3fPtr(ambiente,luzes.ambiente.x,luzes.ambiente.y,luzes.ambiente.z);
        glUniform3fPtr(natural,luzes.natural.x,luzes.natural.y,luzes.natural.z);
        glUniform3fPtr(artificial,luzes.artificial.x,luzes.artificial.y,luzes.artificial.z);
        Vec3 luzJardim{1.12f,1.12f,1.05f};
        if (cena.periodo==DayPeriod::Afternoon) luzJardim={1.05f,.72f,.49f};
        if (cena.periodo==DayPeriod::Night) luzJardim={.16f,.23f,.36f};
        glUniform3fPtr(luzExterna,luzJardim.x,luzJardim.y,luzJardim.z);
        std::vector<const Part*> transparentes;
        for (const auto& parte : cena.partes) {
            if (!visiblePart(cena,parte)) continue;
            if (materiais[static_cast<int>(parte.material)].alpha<1) transparentes.push_back(&parte);
            else drawPart(cena,parte);
        }
        auto distancia=[&](const Part* p) {
            Vec3 deslocamento=transformPoint(partModel(cena,*p),{0,0,0})-camera;
            return dot(deslocamento,deslocamento);
        };
        std::sort(transparentes.begin(),transparentes.end(),[&](const Part* a,const Part* b){return distancia(a)>distancia(b);});
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        for (const Part* parte : transparentes) drawPart(cena,*parte);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    void drawHud(int largura, int altura, const Scene& cena, Interaction alvo) {
        const auto vertices=makeHud(largura,altura,cena,alvo);
        const auto unidade=identity();
        glDisable(GL_DEPTH_TEST);
        glUniformMatrix4fvPtr(projecao,1,GL_FALSE,unidade.valores.data());
        glUniformMatrix4fvPtr(visao,1,GL_FALSE,unidade.valores.data());
        glUniformMatrix4fvPtr(matrizModelo,1,GL_FALSE,unidade.valores.data());
        glUniform4fPtr(cor,1,1,1,1);
        glUniform3fPtr(material,0,1,1);
        glUniform1iPtr(externo,0);
        glUniform1iPtr(vidro,0);
        glBindTexture(GL_TEXTURE_2D,texturas[static_cast<int>(TextureKind::White)]);
        glBindVertexArrayPtr(painel.vao);
        glBindBufferPtr(GL_ARRAY_BUFFER,painel.vbo);
        glBufferDataPtr(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(Vertex)),vertices.data(),GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES,0,static_cast<GLsizei>(vertices.size()));
        glEnable(GL_DEPTH_TEST);
    }

    void release() {
        glDeleteBuffersPtr(1,&painel.vbo);
        glDeleteVertexArraysPtr(1,&painel.vao);
        for (auto malha : malhas) {
            glDeleteBuffersPtr(1,&malha.vbo);
            glDeleteVertexArraysPtr(1,&malha.vao);
        }
        glDeleteTextures(static_cast<GLsizei>(texturas.size()),texturas.data());
        glDeleteTextures(1,&mapaSol.texture);
        glDeleteTextures(1,&mapaLuminaria.texture);
        glDeleteFramebuffersPtr(1,&mapaSol.bufferQuadro);
        glDeleteFramebuffersPtr(1,&mapaLuminaria.bufferQuadro);
        glDeleteProgramPtr(programaProfundidade);
        glDeleteProgramPtr(programa);
    }
};

// Capturas do bufferQuadro permitem conferir a cena sem depender do foco do mouse.
void saveFrame(const char* nome, int largura, int altura) {
    const int passoLinha=(largura*3+3)&~3;
    std::vector<unsigned char> pixels(passoLinha*altura);
    glPixelStorei(GL_PACK_ALIGNMENT,4);
    glReadBuffer(GL_BACK);
    glReadPixels(0,0,largura,altura,GL_BGR,GL_UNSIGNED_BYTE,pixels.data());
    BITMAPFILEHEADER arquivo{};
    BITMAPINFOHEADER informacoes{};
    arquivo.bfType=0x4D42;
    arquivo.bfOffBits=sizeof(arquivo)+sizeof(informacoes);
    arquivo.bfSize=arquivo.bfOffBits+static_cast<DWORD>(pixels.size());
    informacoes.biSize=sizeof(informacoes); informacoes.biWidth=largura; informacoes.biHeight=altura;
    informacoes.biPlanes=1; informacoes.biBitCount=24; informacoes.biCompression=BI_RGB;
    std::ofstream out(nome,std::ios::binary);
    out.write(reinterpret_cast<const char*>(&arquivo),sizeof(arquivo));
    out.write(reinterpret_cast<const char*>(&informacoes),sizeof(informacoes));
    out.write(reinterpret_cast<const char*>(pixels.data()),static_cast<std::streamsize>(pixels.size()));
    if (!out) throw std::runtime_error(std::string("Falha ao salvar captura: ")+nome);
}

HWND janela = nullptr;
bool executando = true;
bool comFoco = true;
bool cursorCapturado = false;
bool redefinirMouse = true;
int larguraJanela = 1280;
int alturaJanela = 720;
std::vector<char> comandos;

void captureCursor(bool capturar) {
    if (capturar == cursorCapturado) return;
    ShowCursor(capturar ? FALSE : TRUE);
    cursorCapturado = capturar;
    if (capturar) redefinirMouse = true;
}

LRESULT CALLBACK windowProc(HWND hwnd, UINT mensagem, WPARAM wParam, LPARAM lParam) {
    switch (mensagem) {
    case WM_SIZE:
        larguraJanela = LOWORD(lParam);
        alturaJanela = HIWORD(lParam);
        redefinirMouse = true;
        return 0;
    case WM_MOVE:
        redefinirMouse = true;
        return 0;
    case WM_SETFOCUS:
        comFoco = true;
        captureCursor(true);
        return 0;
    case WM_KILLFOCUS:
        comFoco = false;
        comandos.clear();
        captureCursor(false);
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) executando = false;
        if (!(lParam & (1LL<<30)) && (wParam=='E' || (wParam>='1' && wParam<='3')))
            comandos.push_back(static_cast<char>(wParam));
        return 0;
    case WM_CLOSE:
        executando = false;
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, mensagem, wParam, lParam);
    }
}

void centerCursor() {
    RECT cliente{};
    GetClientRect(janela, &cliente);
    POINT centro{(cliente.right - cliente.left) / 2, (cliente.bottom - cliente.top) / 2};
    ClientToScreen(janela, &centro);
    SetCursorPos(centro.x, centro.y);
}

void verifyWindowEvents() {
    comandos.clear();
    PostMessageW(janela,WM_KEYDOWN,'E',0);
    PostMessageW(janela,WM_KEYDOWN,'E',1LL<<30); // Autorrepeticao deve ser ignorada.
    for (char tecla : {'2','3','1'}) PostMessageW(janela,WM_KEYDOWN,tecla,0);
    MSG mensagem{};
    while (PeekMessageW(&mensagem,nullptr,0,0,PM_REMOVE)) DispatchMessageW(&mensagem);
    if (std::string(comandos.begin(),comandos.end())!="E231")
        throw std::runtime_error("Falha no despacho de teclas/autorrepeticao Win32.");
    Scene sonda=makeScene();
    Vec3 camera{7,1.65f,4.2f}, alvo{8.46f,1.1f,4.075f};
    for (char tecla : comandos) applyCommand(sonda,tecla,camera,normalized(alvo-camera));
    if (!sonda.portaAberta || sonda.periodo!=DayPeriod::Morning)
        throw std::runtime_error("Eventos Win32 nao chegaram as acoes.");
    comandos.clear();
    RECT antes{},cliente{};
    GetWindowRect(janela,&antes);
    MoveWindow(janela,antes.left,antes.top,820,580,FALSE);
    GetClientRect(janela,&cliente);
    if (larguraJanela!=cliente.right || alturaJanela!=cliente.bottom || !redefinirMouse)
        throw std::runtime_error("Redimensionamento nao atualizou o viewport.");
    MoveWindow(janela,antes.left,antes.top,antes.right-antes.left,antes.bottom-antes.top,FALSE);
    comandos.push_back('E');
    SendMessageW(janela,WM_KILLFOCUS,0,0);
    if (!comandos.empty() || cursorCapturado)
        throw std::runtime_error("Perda de foco nao limpou entrada/cursor.");
    std::cout << "OK: eventos Win32 E/1/2/3, autorrepeticao, tamanho e perda de foco.\n";
}

int run(bool isExecucaoEmModoTeste) {
    SetProcessDPIAware();

    // em caso de teste ajusta os tamanhos para um padrao
    if (isExecucaoEmModoTeste) { 
        larguraJanela=960; 
        alturaJanela=640; 
    }

    // registra e cria a janela principal do Windows.
    const HINSTANCE instancia = GetModuleHandleW(nullptr);
    WNDCLASSW classeJanela{};
    classeJanela.lpfnWndProc = windowProc;
    classeJanela.hInstance = instancia;
    classeJanela.lpszClassName = L"QuartoOpenGL";
    classeJanela.hCursor = LoadCursor(nullptr, IDC_ARROW);
    if (!RegisterClassW(&classeJanela)) throw std::runtime_error("Falha ao registrar a janela.");

    const DWORD estilo = WS_OVERLAPPEDWINDOW;
    RECT limites{0, 0, larguraJanela, alturaJanela};
    AdjustWindowRect(&limites, estilo, FALSE);
    janela = CreateWindowExW(0, classeJanela.lpszClassName, L"Quarto Interativo",
                             estilo, CW_USEDEFAULT, CW_USEDEFAULT,
                             limites.right - limites.left, limites.bottom - limites.top,
                             nullptr, nullptr, instancia, nullptr);
    if (!janela) throw std::runtime_error("Falha ao criar a janela.");


    // Prepara janela para renderizacao e cria contexto Open GL 3.3
    // ele seta: Versão do GL, cria ambiente usado para 
    // gerenciar shaders, texturas, malhas, buffers e os demais estods e recursos graficos da GPU, que serão criados e associados posteriormente
    HDC contextoDispositivo = GetDC(janela);
    PIXELFORMATDESCRIPTOR descritorFormatoPixel{};
    descritorFormatoPixel.nSize = sizeof(descritorFormatoPixel);
    descritorFormatoPixel.nVersion = 1;
    descritorFormatoPixel.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    descritorFormatoPixel.iPixelType = PFD_TYPE_RGBA;
    descritorFormatoPixel.cColorBits = 32;
    descritorFormatoPixel.cDepthBits = 24;
    descritorFormatoPixel.iLayerType = PFD_MAIN_PLANE;
    const int formatoPixel = ChoosePixelFormat(contextoDispositivo, &descritorFormatoPixel);

    if (!formatoPixel || !SetPixelFormat(contextoDispositivo, formatoPixel, &descritorFormatoPixel))
        throw std::runtime_error("Falha ao configurar o formato de pixels.");

    HGLRC contextoTemporario = wglCreateContext(contextoDispositivo);
    if (!contextoTemporario || !wglMakeCurrent(contextoDispositivo, contextoTemporario))
        throw std::runtime_error("Falha ao criar o contexto OpenGL temporario.");

    auto criarContexto = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(
        glFunction("wglCreateContextAttribsARB"));

    if (!criarContexto)
        throw std::runtime_error("OpenGL 3.3 nao disponivel neste computador.");

    const int atributos[] = {WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 3, WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0};
    HGLRC contexto = criarContexto(contextoDispositivo, nullptr, atributos);
    if (!contexto) throw std::runtime_error("Falha ao criar contexto OpenGL 3.3.");
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(contextoTemporario);

    if (!wglMakeCurrent(contextoDispositivo, contexto))
        throw std::runtime_error("Falha ao ativar contexto OpenGL 3.3.");

    loadOpenGL();

    auto intervaloTroca = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(
        glFunction("wglSwapIntervalEXT"));

    if (intervaloTroca) intervaloTroca(1);



    // Cria o estado inicial da cena e os recursos usados pelo renderizador.
    Scene cena=makeScene();
    Renderer renderizador;
    glClearColor(0.43f, 0.67f, 0.87f, 1.0f);

    // Prepara os quadros usados pelo modo de verificacao automatica.
    std::size_t quadroVerificacao=0;
    std::vector<VerificationFrame> quadrosVerificacao;
    if (isExecucaoEmModoTeste) {
        verifyWindowEvents();
        quadrosVerificacao=makeVerificationFrames();
    }

    // Posiciona e orienta a camera no ponto inicial da experiencia.
    Vec3 camera=cameraInicial;
    const Vec3 direcaoInicial=normalized(alvoInicial-camera);
    float guinada=std::atan2(direcaoInicial.z,direcaoInicial.x);
    float inclinacao=std::asin(direcaoInicial.y);

    // Exibe a janela e captura o cursor apenas durante o uso interativo.
    if (!isExecucaoEmModoTeste) {
        ShowWindow(janela, SW_SHOW);
        UpdateWindow(janela);
        captureCursor(true);
        centerCursor();
    }

    // Inicializa a medicao de tempo e apresenta os controles no terminal.
    auto anterior = std::chrono::steady_clock::now();
    std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';
    std::cout << "W/A/S/D: mover | mouse: olhar | E: interagir | 1/2/3: horario | Esc: sair\n";

    // Mantem a aplicacao em execucao, processando entrada, simulacao e desenho.
    while (executando) {
        // Processa todas as mensagens pendentes enviadas pelo Windows.
        MSG mensagem{};
        while (PeekMessageW(&mensagem, nullptr, 0, 0, PM_REMOVE)) {
            if (mensagem.message == WM_QUIT) executando = false;
            TranslateMessage(&mensagem);
            DispatchMessageW(&mensagem);
        }
        if (!executando) break;

        // Calcula o tempo transcorrido desde o quadro anterior.
        const auto agora = std::chrono::steady_clock::now();
        const float dt = std::min(std::chrono::duration<float>(agora - anterior).count(), 0.05f);
        anterior = agora;

        // Le o mouse e o teclado quando a janela esta no modo interativo.
        if (!isExecucaoEmModoTeste && comFoco && cursorCapturado) {
            // Converte o deslocamento do mouse em rotacao da camera.
            if (redefinirMouse) {
                centerCursor();
                redefinirMouse = false;
            } else {
                RECT cliente{};
                GetClientRect(janela, &cliente);
                POINT centro{(cliente.right - cliente.left) / 2,
                             (cliente.bottom - cliente.top) / 2};
                ClientToScreen(janela, &centro);
                POINT mouse{};
                GetCursorPos(&mouse);
                guinada += static_cast<float>(mouse.x - centro.x) * 0.0025f;
                inclinacao = std::clamp(inclinacao - static_cast<float>(mouse.y - centro.y) * 0.0025f,
                                   -1.48f, 1.48f);
                SetCursorPos(centro.x, centro.y);
            }

            // Converte as teclas WASD em uma direcao de movimento.
            const Vec3 frente{std::cos(guinada), 0, std::sin(guinada)};
            const Vec3 direita{-frente.z, 0, frente.x};
            Vec3 direcao{0, 0, 0};
            auto pressionada = [](int tecla) { return (GetAsyncKeyState(tecla) & 0x8000) != 0; };
            if (pressionada('W')) direcao = direcao + frente;
            if (pressionada('S')) direcao = direcao - frente;
            if (pressionada('D')) direcao = direcao + direita;
            if (pressionada('A')) direcao = direcao - direita;
            moveCamera(cena,camera,direcao,dt);

            // Aplica interacoes pendentes e atualiza a simulacao da cena.
            Vec3 direcaoOlhar{std::cos(guinada)*std::cos(inclinacao),std::sin(inclinacao),std::sin(guinada)*std::cos(inclinacao)};
            for (char tecla : comandos) applyCommand(cena,tecla,camera,direcaoOlhar);
            updateScene(cena,dt,camera);
        }

        // Descarta os comandos que ja foram tratados neste quadro.
        comandos.clear();

        // Carrega o estado e a camera do quadro atual de verificacao.
        if (isExecucaoEmModoTeste) {
            const auto& captura=quadrosVerificacao[quadroVerificacao];
            cena=captura.cena;
            camera=captura.camera;
            Vec3 direcao=normalized(captura.alvo-camera);
            guinada=std::atan2(direcao.z,direcao.x);
            inclinacao=std::asin(direcao.y);
        }

        // Desenha a cena quando a janela possui uma area visivel valida.
        if (larguraJanela > 0 && alturaJanela > 0) {
            glViewport(0, 0, larguraJanela, alturaJanela);
            const Lighting luzes=lightingFor(cena);
            glClearColor(luzes.ceu.x,luzes.ceu.y,luzes.ceu.z,1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            const Mat4 projecao = perspective(70.0f * pi / 180.0f,
                                                float(larguraJanela) / float(alturaJanela),
                                                0.05f, 50.0f);
            const Vec3 direcaoOlhar{std::cos(guinada) * std::cos(inclinacao), std::sin(inclinacao),
                                     std::sin(guinada) * std::cos(inclinacao)};
            const Mat4 visao = lookAt(camera, camera + direcaoOlhar);
            renderizador.draw(cena,camera,projecao,visao,larguraJanela,alturaJanela);
            renderizador.drawHud(larguraJanela,alturaJanela,cena,aimedInteraction(cena,camera,direcaoOlhar));

            // Salva e registra o resultado do quadro no modo de verificacao.
            if (isExecucaoEmModoTeste) {
                saveFrame(quadrosVerificacao[quadroVerificacao].arquivo.c_str(),larguraJanela,alturaJanela);
                if (glGetError()!=GL_NO_ERROR) throw std::runtime_error("Erro OpenGL durante verificacao.");
                std::cout << "Captura: " << quadrosVerificacao[quadroVerificacao].arquivo
                          << " | porta=" << cena.anguloPorta << " janela=" << cena.anguloJanela
                          << " bola=" << cena.posicaoBola.x << "," << cena.posicaoBola.z
                          << " giro=" << cena.rotacaoBola << " | " << statusText(cena) << std::endl;
                ++quadroVerificacao;
                if (quadroVerificacao==quadrosVerificacao.size()) executando=false;
            }

            // Apresenta na janela o quadro que acabou de ser desenhado.
            SwapBuffers(contextoDispositivo);
        } else {
            // Evita uso desnecessario do processador enquanto a janela esta minimizada.
            Sleep(16);
        }
    }

    // Libera os recursos da aplicacao na ordem inversa da inicializacao.
    captureCursor(false);
    renderizador.release();
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(contexto);
    ReleaseDC(janela, contextoDispositivo);
    DestroyWindow(janela);
    return 0;
}

} 


// inicializador da aplicacao
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
