#pragma once

#include "vec2.hpp"
#include "vec2_qt.hpp"

#include <QTransform> // TODO: Replace with a custom matrix type

#include <cassert>
#include <iterator>
#include <numbers>
// #include <ranges>
#include <span>
#include <vector>

namespace detail {

constexpr inline struct EndIterTag final {} EndIter;

} // namespace detail


template <typename Impl>
class FractalIterator final
{
public:
    using Self = FractalIterator<Impl>;

    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = const Vec2d;
    using pointer           = const Vec2d*;
    using reference         = const Vec2d&;

    template <typename... Args>
    FractalIterator(Args&&... args):
        impl_{ std::forward<Args>(args)... }
    {}

    FractalIterator(const FractalIterator&) = default;
    FractalIterator(FractalIterator&&) = default;

    reference operator*() const
    { return impl_.deref(); }

    pointer operator->() const
    { return &impl_.deref(); }

    // Prefix increment
    Self& operator++()
    { impl_.inc(); return *this; }

    // Postfix increment
    Self operator++(int)
    { Self tmp = *this; ++(*this); return tmp; }

    friend bool operator== (const Self& a, const Self& b)
    { return a.impl_.equal(b.impl_); }

    friend bool operator!= (const Self& a, const Self& b)
    { return !a.impl_.equal(b.impl_); }

private:
    Impl impl_;
};


class FractalNGen final
{
public:

    FractalNGen(std::span<const Vec2d> base,
                std::span<const Vec2d> generator,
                size_t generation):
        base_{ base },
        generator_{ generator },
        generation_{ generation },
        genProp_{ lengthAndAngle( generator_.back() - generator_.front()) },
        value_{ base.front() }
    {
        assert(base_.size() > 1);
        assert(generator_.size() > 1);
        state_.reserve(generation + 1);

        state_.push_back(baseState());
        for (size_t gen=0; gen<generation_; ++gen)
            state_.push_back(recurseState(state_.back()));
    }

    FractalNGen(detail::EndIterTag):
        is_end_{ true }
    {}

    auto deref() const noexcept
        -> const Vec2d&
    { return value_; }

    auto inc() noexcept
        -> void
    {
        assert(!is_end_);
        ++ordinal_;

        if (is_last_)
        {
            is_end_ = true;
            return;
        }

        auto gen = generation_;
        for (; gen!=~0ul; --gen)
        {
            auto& st = state_[gen];
            if (!st.is_last())
            {
                st.next();
                for(; gen<generation_; ++gen)
                    state_[gen+1] = recurseState(state_[gen]);
                value_ = state_.back().v0;
                return;
            }
        }

        value_ = state_.front().v1;
        is_last_ = true;
    }

    auto equal(const FractalNGen& that) const noexcept
        -> bool
    {
        if (is_end_ != that.is_end_)
            return false;
        if (is_end_)
            return true;

        return ordinal_ == that.ordinal_;
    }

private:
    struct GenerationState final
    {
        const Vec2d* begin;
        const Vec2d* end;
        Vec2d v0;
        Vec2d v1;
        QTransform transform;

        auto is_last() const noexcept
            -> bool
        { return begin + 1 == end; }

        auto next() noexcept
            -> void
        {
            assert(!is_last());
            v0 = v1;
            ++begin;
            if (begin != end)
                v1 = toVec2d(transform.map(toQPointF(begin[1])));
        }
    };

    struct LengthAndAngle final
    {
        double length;
        double angle;
    };

    static auto lengthAndAngle(const Vec2d& dr) noexcept
        -> LengthAndAngle
    { return { dr.norm(), atan2(dr[1], dr[0]) }; }

    auto baseState() const
        -> GenerationState
    {
        return{
            .begin = base_.data(),
            .end = base_.data() + base_.size() - 1,
            .v0 = base_[0],
            .v1 = base_[1],
            .transform = {}
        };
    }

    auto recurseState(const GenerationState& state) const
        -> GenerationState
    {
        // deBUG, TODO: Remove
        [[maybe_unused]]
        auto gsize = generator_.size();
        [[maybe_unused]]
        auto bsize = base_.size();

        auto baseProp = lengthAndAngle(state.v1 - state.v0);
        auto scale = baseProp.length / genProp_.length;
        auto angle = baseProp.angle - genProp_.angle;
        const auto& g0 = generator_[0];
        const auto& g1 = generator_[1];
        auto t = QTransform{};
        t
            .translate(state.v0[0], state.v0[1])
            .scale(scale, scale)
            .rotate(angle / std::numbers::pi_v<double> * 180)
            .translate(-g0[0], -g0[1]);
        return {
            .begin = generator_.data(),
            .end = generator_.data() + generator_.size() - 1,
            .v0 = state.v0,
            .v1 = toVec2d(t.map(toQPointF(g1))),
            .transform = t
        };
    }

    std::span<const Vec2d> base_;
    std::span<const Vec2d> generator_;
    size_t generation_{};
    size_t ordinal_{};
    bool is_last_{ false };
    bool is_end_{ false };

    LengthAndAngle genProp_;

    Vec2d value_;

    std::vector<GenerationState> state_;
};

using FractalNGenIterator =
    FractalIterator<FractalNGen>;

template <typename Iterator>
struct Range {
    Iterator begin_;
    Iterator end_;

    // Provide begin() and end() methods
    Iterator begin() const { return begin_; }
    Iterator end() const { return end_; }
};

template <typename Impl, typename... Args>
auto fractalSeq(Args&&... args)
{
    using Iter = FractalIterator<Impl>;
    return Range{ Iter(std::forward<Args>(args)...),
                  Iter(detail::EndIter) };
}

// TODO
// template <typename Impl, typename... Args>
// auto fractal_seq(Args&&... args)
// {
//     using Iter = FractalIterator<Impl>;
//     return std::ranges::subrange(Iter(std::forward<Args>(args)...),
//                                  Iter(detail::EndIter));
// }
