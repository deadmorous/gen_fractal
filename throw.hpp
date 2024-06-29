#pragma once

#include <sstream>
#include <stdexcept>
#include <utility>

template <typename Exception = std::runtime_error, typename... Args>
[[noreturn]]
auto throw_(Args&&... args)
    -> void
{
    std::ostringstream s;
    ((s << std::forward<Args>(args)), ...);
    throw Exception{ s.str() };
}
