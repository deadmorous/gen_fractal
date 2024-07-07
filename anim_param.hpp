#pragma once

#include "type.hpp"

#include <array>
#include <cstddef>
#include <string_view>
#include <tuple>

struct AnimParam
{
    size_t frameCountAfter{25};
    bool reflectX{false};
    bool reflectY{false};
    double slopeFactor{1};
};


inline auto field_names_of(TypeTag<AnimParam>)
    -> std::array<std::string_view, 4>
{ return { "frames_after", "reflect_x", "reflect_y", "slopeFactor" }; }

inline auto fields_of(AnimParam& p)
    -> std::tuple<size_t&, bool&, bool&, double&>
{ return std::tie(p.frameCountAfter, p.reflectX, p.reflectY, p.slopeFactor); }

inline auto fields_of(const AnimParam& p)
    -> std::tuple<const size_t&, const bool&, const bool&, const double&>
{ return std::tie(p.frameCountAfter, p.reflectX, p.reflectY, p.slopeFactor); }
