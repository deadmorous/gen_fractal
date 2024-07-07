#pragma once

#include "cubic.hpp"

#include <cassert>
#include <cstdlib>
#include <ranges>

inline auto localIndex(size_t iglobal, size_t nglobal, size_t nlocal)
    -> size_t
{
    assert(iglobal < nglobal);
    assert(nlocal <= nglobal);

    // NOTE
    //      We want symmetry in the interpolated curve if we have it
    //      in both original curves. For that to work, index mapping
    //      needs to possess a symmetry too.
    //      Otherwise, we could simply `return iglobal * nlocal / nglobal`;
    if (((nlocal ^ nglobal) & 1) != 0 && nlocal*2 > nglobal)
    {
        // This branch is necessary to avoid skipping local index
        // in the middle of the range
        if (2*iglobal < nglobal)
        {
            ++iglobal;
            return iglobal * nlocal / nglobal;
        }
        else
        {
            --iglobal;
            return nlocal - 1 - (nglobal - 1 - iglobal) * nlocal / nglobal;
        }

        if (iglobal*2 < nglobal)
            ++iglobal;
    }
    else if (2*iglobal < nglobal)
        return iglobal * nlocal / nglobal;
    else
        return nlocal - 1 - (nglobal - 1 - iglobal) * nlocal / nglobal;
}

template <typename Keyframe, typename KeyframeCurves>
using PointOf =
    std::remove_cvref_t<
        decltype(std::declval<KeyframeCurves>()(std::declval<Keyframe>())[0]) >;


template <typename Keyframe,
          typename KeyframeCurves,
          typename KeyframeSlopeF,
          typename KeyframeSubdivAfter>
auto interpCurves(std::span<const Keyframe> keyframes,
                  KeyframeCurves keyframeCurves,
                  KeyframeSubdivAfter subdivAfter,
                  KeyframeSlopeF keyframeSlopeF)
    -> std::vector<std::vector< PointOf<Keyframe, KeyframeCurves> >>
{
    using Point =
        PointOf<Keyframe, KeyframeCurves>;

    using Result =
        std::vector<std::vector<Point>>;

    auto kfsCurves =
        std::ranges::transform_view(keyframes, keyframeCurves);

    auto kfsCurveCount =
        std::ranges::transform_view(
            kfsCurves,
            [](const auto& kfCurves){ return kfCurves.size(); });

    // Compute the total (maximal over keyframes) number of curves
    auto nglobal =
        *std::max_element(
            kfsCurveCount.begin(), kfsCurveCount.end() );

    // Compute curve nodes
    auto curves = Result( nglobal );
    for (const auto& kf: keyframes)
    {
        const auto& kfCurves = keyframeCurves(kf);
        auto nlocal = kfCurves.size();
        for (size_t iglobal=0; iglobal<nglobal; ++iglobal)
        {
            auto ilocal = localIndex(iglobal, nglobal, nlocal);
            curves[iglobal].push_back( kfCurves[ilocal] );
        }
    }

    auto nframes = keyframes.size();

    // Compute original slopes
    auto slopes = Result{};
    slopes.reserve(nglobal);
    for (size_t iglobal=0; iglobal<nglobal; ++iglobal)
    {
        slopes.push_back( cubicSlopes<Point>(curves[iglobal]) );
        // auto m = cubicSlopes<Point>(curves[iglobal]);
        // for (size_t iframe=0; iframe<nframes; ++iframe)
        //     m[iframe] *= keyframeSlopeF(keyframes[iframe]);
        // slopes.push_back( m );
    }

    // Average slopes over collapsing curves
    for (size_t iframe=0; iframe<nframes; ++iframe)
    {
        auto nlocal = keyframeCurves(keyframes[iframe]).size();
        if (nlocal == nglobal)
            continue;

        auto msum = Point{};
        size_t iglobalBegin = 0;
        auto flush = [&](size_t iglobalEnd)
        {
            auto m = msum / (iglobalEnd - iglobalBegin);
            for (auto iglobal=iglobalBegin; iglobal!=iglobalEnd; ++iglobal)
                slopes[iglobal][iframe] = m;
            msum = Point{};
            iglobalBegin = iglobalEnd;
        };

        size_t ilocalPrev = 0;
        for (size_t iglobal=0; iglobal<nglobal; ++iglobal)
        {
            auto ilocal = localIndex(iglobal, nglobal, nlocal);
            if (ilocal != ilocalPrev)
                flush(iglobal);
            ilocalPrev = ilocal;
            msum += slopes[iglobal][iframe];
        }
        flush(nglobal);
    }

    // Interpolate result
    auto result = Result{};
    result.reserve(nglobal);
    auto slopeF = [&](size_t ikf){ return keyframeSlopeF(keyframes[ikf]); };
    auto subdiv = [&](size_t ikf){ return subdivAfter(keyframes[ikf]); };
    for (size_t iglobal=0; iglobal<nglobal; ++iglobal)
    {
        const auto& y = curves[iglobal];
        const auto& m = slopes[iglobal];
        result.push_back(cubicInterp<Point>(y, m, subdiv, slopeF));
    }

    return result;
}
