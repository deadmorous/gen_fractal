#pragma once

#include <cassert>
#include <span>
#include <vector>

template <typename Point>
auto cubic_slopes(std::span<const Point> y)
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

template <typename Point>
auto cubic_interp(std::span<const Point> y,
                  std::span<const Point> m,
                  size_t subdiv)
    -> std::vector<Point>
{
    size_t n = m.size();
    std::vector<Point> result;
    result.reserve((n - 1)*subdiv + 1);

    result.push_back(y[0]);

    for (size_t piece=0; piece+1<n; ++piece)
    {
        for (size_t i=1; i<=subdiv; ++i)
        {
            auto xi = static_cast<double>(i) / subdiv;
            auto xi2 = xi * xi;
            auto xi3 = xi2 * xi;
            auto f1 = 1 - 3*xi2 + 2*xi3;
            auto f2 = 1 - f1;
            auto f3 = xi - 2*xi2 + xi3;
            auto f4 = xi3 - xi2;
            result.push_back(
                f1*y[piece] + f2*y[piece+1] +
                f3*m[piece] + f4*m[piece+1]);
        }
    }
    return result;
}
