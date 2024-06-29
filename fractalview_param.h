#pragma once

#include <array>
#include <cstddef>
#include <string_view>
#include <tuple>

struct FractalViewParam
{
    size_t generations{5};
    bool antialiasing{true};
    bool fancyPen{true};
    bool allGenerations{true};

    bool approxAlgorithm{false};

    size_t approxAlgorithmBboxGen{5};
    size_t approxAlgorithmMaxGen{30};
    size_t approxAlgorithmMaxVertexCount{10'000'000};

    double adjustScale{1};
};

inline auto fractalViewParamFieldNames()
    -> std::array<std::string_view, 9>
{
    return {
        "gen",
        "antialiasing",
        "fancy_pen",
        "all_gen",
        "approx",
        "approx_bbox_gen",
        "approx_max_gen",
        "approx_max_vert",
        "adjust_scale"
    };
}

inline auto fields_of(FractalViewParam& p)
    -> std::tuple<
        size_t&,
        bool&,
        bool&,
        bool&,
        bool&,
        size_t&,
        size_t&,
        size_t&,
        double&>
{
    return std::tie(
        p.generations,
        p.antialiasing,
        p.fancyPen,
        p.allGenerations,
        p.approxAlgorithm,
        p.approxAlgorithmBboxGen,
        p.approxAlgorithmMaxGen,
        p.approxAlgorithmMaxVertexCount,
        p.adjustScale );
}

inline auto fields_of(const FractalViewParam& p)
    -> std::tuple<
        const size_t&,
        const bool&,
        const bool&,
        const bool&,
        const bool&,
        const size_t&,
        const size_t&,
        const size_t&,
        const double&>
{
    return std::tie(
        p.generations,
        p.antialiasing,
        p.fancyPen,
        p.allGenerations,
        p.approxAlgorithm,
        p.approxAlgorithmBboxGen,
        p.approxAlgorithmMaxGen,
        p.approxAlgorithmMaxVertexCount,
        p.adjustScale );
}
