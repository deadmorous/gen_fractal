#pragma once

#include "vec2.hpp"
#include "fractalview_param.h"

#include <QRect>

#include <chrono>
#include <span>

class QPainter;

struct RenderFractlalResult
{
    std::chrono::nanoseconds computeBbTime{};
    std::chrono::nanoseconds renderTime{};
    size_t vertexCount{};
    size_t maxGen{};
    double scale{};
};

auto renderFractal(QPainter& painter,
                   const QRect& rect,
                   std::span<const Vec2d> base,
                   std::span<const Vec2d> gen,
                   const FractalViewParam& param)
    -> RenderFractlalResult;
