#pragma once

#include <cassert>
#include <span>
#include <vector>

template <typename Point>
auto cubicSlopes(std::span<const Point> y)
    -> std::vector<Point>
{
    auto n = y.size();
    assert(n > 1);
    std::vector<double> b( n, 4. );
    b.front() = b.back() = 2;
    std::vector<Point> f(n);

    f.front() = 3*(y[1] - y[0]);
    for (size_t i=1; i+1<n; ++i)
        f[i] = 3*(y[i+1] - y[i-1]);
    f.back() = 3*(y[n-1] - y[n-2]);

    for (size_t i=1; i<n; ++i)
    {
        auto k = 1. / b[i-1];
        b[i] -= k;
        f[i] -= k*f[i-1];
    }

    auto m = std::vector<Point>( n );
    m.back() = f.back() / b.back();

    for (size_t i=n-2; i!=~0ul; --i)
        m[i] = (f[i] - m[i+1]) / b[i];

    return m;
}

template <typename Point, typename SlopeF, typename Subdiv>
auto cubicInterp(std::span<const Point> y,
                 std::span<const Point> m,
                 Subdiv subdiv,
                 SlopeF slopeF)
    -> std::vector<Point>
{
    size_t n = m.size();
    std::vector<Point> result;

    struct Fs
    {
        double f1;
        double f2;
        double f3;
        double f4;
    };

    auto fs = [](double xi) -> Fs
    {
        auto xi2 = xi * xi;
        auto xi3 = xi2 * xi;
        auto f1 = 1 - 3*xi2 + 2*xi3;
        auto f2 = 1 - f1;
        auto f3 = xi - 2*xi2 + xi3;
        auto f4 = xi3 - xi2;
        return { f1, f2, f3, f4 };
    };

    auto interp =
        []<typename T>(const Fs& fs,
                       const T& y0, const T& y1,
                       const T& m0, const T& m1) -> T
    { return fs.f1*y0 + fs.f2 * y1 + fs.f3*m0 + fs.f4*m1; };

    result.push_back(y[0]);

    auto sf0 = slopeF(0);
    for (size_t piece=0; piece+1<n; ++piece)
    {
        auto sf1 = slopeF(piece+1);
        auto subdivVal = subdiv(piece);
        for (size_t i=1; i<=subdivVal; ++i)
        {
            auto fxi = fs(static_cast<double>(i) / subdivVal);
            auto p = interp(fxi, 0., 1., sf0, sf1);
            auto fp = fs(p);
            result.push_back(
                interp(fp, y[piece], y[piece+1], m[piece], m[piece+1]));
        }
        sf0 = sf1;
    }
    return result;
}

template <typename Point>
auto cubicInterp(std::span<const Point> y,
                 std::span<const Point> m,
                 size_t subdiv)
    -> std::vector<Point>
{
    return cubicInterp(
        y,
        m,
        [subdiv](size_t)->double{ return subdiv; },
        [](size_t)->double{ return 1; });
}
