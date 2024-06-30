#pragma once

template <typename T>
struct TypeTag final {};

template <typename T>
constexpr inline auto Type = TypeTag<T>{};
