#pragma once
#include "scene.hpp"

// O emissor proximo ao quarto orienta a luz e o mapa de sombras.
// O astro visivel fica mais longe, no mesmo raio visto do centro do quarto.
inline Vec3 naturalPosition(DayPeriod period) {
    switch (period) {
    case DayPeriod::Morning:   return {-2.5f,2.5f,2.48f};
    case DayPeriod::Afternoon: return {11.f,2.45f,4.08f};
    case DayPeriod::Night:     return {-2.5f,2.45f,2.75f};
    }
    return {-2.5f,2.5f,2.48f};
}

inline Vec3 celestialVisualPosition(DayPeriod period) {
    constexpr Vec3 roomCenter{roomWidth*.5f,1.f,roomDepth*.5f};
    return roomCenter + (naturalPosition(period)-roomCenter)*2.6f;
}
