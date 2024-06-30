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
};


inline auto field_names_of(TypeTag<AnimParam>)
    -> std::array<std::string_view, 3>
{ return { "frames_after", "reflect_x", "reflect_y" }; }

inline auto fields_of(AnimParam& p)
    -> std::tuple<size_t&, bool&, bool&>
{ return std::tie(p.frameCountAfter, p.reflectX, p.reflectY); }

inline auto fields_of(const AnimParam& p)
    -> std::tuple<const size_t&, const bool&, const bool&>
{ return std::tie(p.frameCountAfter, p.reflectX, p.reflectY); }
