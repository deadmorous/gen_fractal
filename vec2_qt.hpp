#pragma once

#include "vec2.hpp"

#include <QPointF>

inline auto toQPointF(const Vec2d& v)
    -> QPointF
{ return { v[0], v[1] }; }

inline auto toVec2d(const QPointF& p)
    -> Vec2d
{ return { p.x(), p.y() }; }

