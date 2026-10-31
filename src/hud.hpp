#pragma once
#include "behavior.hpp"

// Fonte bitmap 5x7 desenhada com triangulos: sem biblioteca de interface.
inline std::array<unsigned char,7> glyph(char c) {
    switch (c) {
    case 'A': return {14,17,17,31,17,17,17}; case 'B': return {30,17,17,30,17,17,30};
    case 'C': return {14,17,16,16,16,17,14}; case 'D': return {30,17,17,17,17,17,30};
    case 'E': return {31,16,16,30,16,16,31}; case 'F': return {31,16,16,30,16,16,16};
    case 'G': return {14,17,16,23,17,17,14}; case 'H': return {17,17,17,31,17,17,17};
    case 'I': return {14,4,4,4,4,4,14}; case 'J': return {7,2,2,2,18,18,12};
    case 'K': return {17,18,20,24,20,18,17}; case 'L': return {16,16,16,16,16,16,31};
    case 'M': return {17,27,21,21,17,17,17}; case 'N': return {17,25,21,19,17,17,17};
    case 'O': return {14,17,17,17,17,17,14}; case 'P': return {30,17,17,30,16,16,16};
    case 'Q': return {14,17,17,17,21,18,13}; case 'R': return {30,17,17,30,20,18,17};
    case 'S': return {15,16,16,14,1,1,30}; case 'T': return {31,4,4,4,4,4,4};
    case 'U': return {17,17,17,17,17,17,14}; case 'V': return {17,17,17,17,17,10,4};
    case 'W': return {17,17,17,21,21,21,10}; case 'X': return {17,17,10,4,10,17,17};
    case 'Y': return {17,17,10,4,4,4,4}; case 'Z': return {31,1,2,4,8,16,31};
    case '0': return {14,17,19,21,25,17,14}; case '1': return {4,12,4,4,4,4,14};
    case '2': return {14,17,1,2,4,8,31}; case '3': return {30,1,1,14,1,1,30};
    case '/': return {1,1,2,4,8,16,16}; case '|': return {4,4,4,4,4,4,4};
    case '-': return {0,0,0,31,0,0,0}; default: return {};
    }
}

inline MeshData makeHud(int width, int height, const Scene& scene, Interaction target) {
    MeshData mesh;
    auto rect=[&](float x,float y,float w,float h,Vec3 color) {
        Vec3 a{2*x/width-1,1-2*y/height,0}, b{2*(x+w)/width-1,1-2*y/height,0};
        Vec3 c{b.x,1-2*(y+h)/height,0}, d{a.x,c.y,0};
        for (auto p : {a,b,c,a,c,d}) mesh.push_back({p,{0,0,1},color,{0,0}});
    };
    auto text=[&](const std::string& value,float x,float y,Vec3 color) {
        for (char c : value) {
            auto bits=glyph(c);
            for (int row=0;row<7;++row) for (int col=0;col<5;++col)
                if (bits[row]&(1<<(4-col))) rect(x+col*2,y+row*2,2,2,color);
            x+=12;
        }
    };
    std::string status=statusText(scene), prompt=interactionPrompt(scene,target);
    rect(12,12,float(status.size()*12+16),30,{.025f,.033f,.043f});
    text(status,20,20,{.72f,.76f,.78f});
    const Vec3 crossColor=target==Interaction::None ? Vec3{.65f,.69f,.70f} : Vec3{1,.80f,.35f};
    float cx=width*.5f, cy=height*.5f;
    rect(cx-6,cy-1,12,2,{.02f,.02f,.02f}); rect(cx-1,cy-6,2,12,{.02f,.02f,.02f});
    rect(cx-5,cy-.5f,10,1,crossColor); rect(cx-.5f,cy-5,1,10,crossColor);
    if (!prompt.empty()) {
        float w=float(prompt.size()*12);
        rect(cx-w*.5f-12,float(height-64),w+24,34,{.025f,.033f,.043f});
        text(prompt,cx-w*.5f,float(height-54),{1,.84f,.47f});
    }
    return mesh;
}
