#pragma once

#include <array>
#include <cmath>
#include <concepts>


template <typename T>
requires (std::floating_point<T> || std::integral<T>)
struct Vec2 : std::array<T, 2>
{
    using V = Vec2<T>;
    using Base = std::array<T, 2>;

    template <typename... Args>
    constexpr Vec2(Args&&... args)
        : Base{ std::forward<Args>(args)... }
    {}

    using Base::operator[];

    friend constexpr auto operator+(const V& a, const V& b) noexcept
        -> V
    { return { a[0] + b[0], a[1] + b[1] }; }

    friend constexpr auto operator-(const V& a, const V& b) noexcept
        -> V
    { return { a[0] - b[0], a[1] - b[1] }; }

    friend constexpr auto operator*(const V& a, const V& b) noexcept
        -> T
    { return a[0] * b[0] + a[1] * b[1]; }

    friend constexpr auto operator%(const V& a, const V& b) noexcept
        -> T
    { return a[0] * b[1] - a[1] * b[0]; }

    friend constexpr auto operator*(const V& a, T f) noexcept
        -> V
    { return { a[0] * f, a[1] * f }; }

    friend constexpr auto operator*(T f, const V& a) noexcept
        -> V
    { return { a[0] * f, a[1] * f }; }

    friend constexpr auto operator/(const V& a, T f) noexcept
        -> V
    { return { a[0] / f, a[1] / f }; }

    auto operator+=(const V& that) noexcept
        -> V&
    { return *this = *this + that; }

    auto operator-=(const V& that) noexcept
        -> V&
    { return *this = *this - that; }

    auto operator*=(T f) noexcept
        -> V&
    { return *this = *this * f; }

    auto operator/=(T f) noexcept
        -> V&
    { return *this = *this / f; }

    auto norm() const noexcept
        -> T
    requires (std::floating_point<T>)
    { return sqrt(*this * *this); }

    auto unit() const noexcept
        -> V
    requires (std::floating_point<T>)
    { return *this / norm(); }
};

using Vec2d = Vec2<double>;
