#pragma once

#include "vec2.hpp"

#include <algorithm>
#include <cassert>


template <typename T>
struct Bbox2
{
    using V = Vec2<T>;

    V min{};
    V max{};
    bool empty{true};

    auto operator<<(const V& v) noexcept
        -> Bbox2<T>&
    {
        if (empty)
        {
            min = max = v;
            empty = false;
        }
        else
        {
            min[0] = std::min(min[0], v[0]);
            min[1] = std::min(min[1], v[1]);
            max[0] = std::max(max[0], v[0]);
            max[1] = std::max(max[1], v[1]);
        }

        return *this;
    }

    auto operator<<(const Bbox2<T>& bb)
        -> Bbox2<T>&
    {
        if (!bb.empty)
            *this << bb.min << bb.max;
        return *this;
    }

    auto size() const noexcept
        -> V
    {
        if (empty)
            return {T{}, T{}};
        else
            return max - min;
    }

    auto size(size_t axis) const noexcept
        -> T
    {
        if (empty)
            return 0;
        else
            return max[axis] - min[axis];
    }

    auto range(size_t axis) const noexcept
        -> V
    {
        if (empty)
            return {T{}, T{}};
        else
            return {min[axis], max[axis]};
    }

    auto center() const noexcept
        -> V
    {
        assert(!empty);
        return 0.5*(min + max);
    }
};

using Bbox2d = Bbox2<double>;
