#pragma once

#include "vec2.hpp"
#include "vec2_qt.hpp"

#include <QTransform> // TODO: Replace with a custom matrix type

#include <cassert>
#include <iterator>
// #include <ranges>
#include <span>
#include <vector>

namespace detail {

constexpr inline struct EndIterTag final {} EndIter;

inline auto generatorTransform(const Vec2d& b0,
                               const Vec2d& b1,
                               const Vec2d& g0,
                               const Vec2d& g1)
    ->QTransform
{
    auto b = b1 - b0;
    auto g = g1 - g0;
    auto ig2 = 1. / (g * g);
    auto fc = (b * g) * ig2;
    auto fs = (b % g) * ig2;
    auto dx = b0[0] - fc*g0[0] - fs*g0[1];
    auto dy = b0[1] + fs*g0[0] - fc*g0[1];
    return QTransform{ fc, -fs, fs, fc, dx, dy };
}

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

    auto impl() const noexcept
        -> const Impl&
    { return impl_; }

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

        if (isLast_)
        {
            is_end_ = true;
            return;
        }

        auto gen = generation_;
        for (; gen!=~0ul; --gen)
        {
            auto& st = state_[gen];
            if (!st.isLast())
            {
                st.next();
                for(; gen<generation_; ++gen)
                    state_[gen+1] = recurseState(state_[gen]);
                value_ = state_.back().v0;
                return;
            }
        }

        value_ = state_.front().v1;
        isLast_ = true;
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

    // ---

    auto actualMaxGen() const noexcept
        -> size_t
    { return generation_; }

private:
    struct GenerationState final
    {
        const Vec2d* begin;
        const Vec2d* end;
        Vec2d v0;
        Vec2d v1;
        QTransform transform;

        auto isLast() const noexcept
            -> bool
        { return begin + 1 == end; }

        auto next() noexcept
            -> void
        {
            assert(!isLast());
            v0 = v1;
            ++begin;
            if (begin != end)
                v1 = toVec2d(transform.map(toQPointF(begin[1])));
        }
    };


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
        auto t = detail::generatorTransform(
            state.v0, state.v1, generator_.front(), generator_.back());
        return {
            .begin = generator_.data(),
            .end = generator_.data() + generator_.size() - 1,
            .v0 = state.v0,
            .v1 = toVec2d(t.map(toQPointF(generator_[1]))),
            .transform = t
        };
    }

    std::span<const Vec2d> base_;
    std::span<const Vec2d> generator_;
    size_t generation_{};
    size_t ordinal_{};
    bool isLast_{ false };
    bool is_end_{ false };

    Vec2d value_;

    std::vector<GenerationState> state_;
};

struct FractalApproxParam final
{
    size_t maxGen{ 30 };
    size_t maxOrdinal{ 10'000'000 };
    double minLength{ 1 };
};

class FractalApprox final
{
public:

    FractalApprox(std::span<const Vec2d> base,
                std::span<const Vec2d> generator,
                const FractalApproxParam& param):
        base_{ base },
        generator_{ generator },
        baseLen_( lengths(base) ),
        genLen_( lengths(generator) ),
        genDist_{ (generator.back() - generator.front()).norm() },
        param_{ param },
        value_{ base.front() }
    {
        assert(base_.size() > 1);
        assert(generator_.size() > 1);
        state_.reserve(param_.maxGen + 1);

        state_.push_back(baseState());
        maybeRecurse();
    }

    FractalApprox(detail::EndIterTag):
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

        if (isLast_)
        {
            is_end_ = true;
            return;
        }

        if (ordinal_ < param_.maxOrdinal)
            while (state_.size() > 1)
            {
                auto& st = state_.back();
                if (st.isLast())
                {
                    state_.pop_back();
                    continue;
                }

                st.next();
                maybeRecurse();
                value_ = state_.back().v0;
                return;
            }

        value_ = state_.front().v1;
        isLast_ = true;
    }

    auto equal(const FractalApprox& that) const noexcept
        -> bool
    {
        if (is_end_ != that.is_end_)
            return false;
        if (is_end_)
            return true;

        return ordinal_ == that.ordinal_;
    }

    // ---

    auto actualMaxGen() const noexcept
        -> size_t
    { return actualMaxGen_; }

private:
    struct GenerationState final
    {
        const Vec2d* begin;
        const Vec2d* end;
        const double* len;
        Vec2d v0;
        Vec2d v1;
        QTransform transform;
        double scale;

        auto isLast() const noexcept
            -> bool
        { return begin + 1 == end; }

        auto next() noexcept
            -> void
        {
            assert(!isLast());
            v0 = v1;
            ++begin;
            ++len;
            if (begin != end)
                v1 = toVec2d(transform.map(toQPointF(begin[1])));
        }

        auto length() const noexcept
        { return scale * *len; }
    };

    auto maybeRecurse()
        -> void
    {
        while (state_.size() < param_.maxGen &&
               state_.back().length() > param_.minLength)
            state_.push_back(recurseState(state_.back()));
        actualMaxGen_ = std::max(actualMaxGen_, state_.size());
    }


    auto baseState() const
        -> GenerationState
    {
        return{
            .begin = base_.data(),
            .end = base_.data() + base_.size() - 1,
            .len = baseLen_.data(),
            .v0 = base_[0],
            .v1 = base_[1],
            .transform = {},
            .scale = 1
        };
    }

    auto recurseState(const GenerationState& state) const
        -> GenerationState
    {
        auto t = detail::generatorTransform(
            state.v0, state.v1, generator_.front(), generator_.back());
        return {
            .begin = generator_.data(),
            .end = generator_.data() + generator_.size() - 1,
            .len = genLen_.data(),
            .v0 = state.v0,
            .v1 = toVec2d(t.map(toQPointF(generator_[1]))),
            .transform = t,
            .scale = state.length() / genDist_
        };
    }

    static auto lengths(std::span<const Vec2d> v)
        -> std::vector<double>
    {
        auto result = std::vector<double>{};
        assert(!v.empty());
        result.reserve(v.size() - 1);
        for (size_t i=1, n=v.size(); i<n; ++i)
            result.push_back((v[i]-v[i-1]).norm());
        return result;
    }

    std::span<const Vec2d> base_;
    std::span<const Vec2d> generator_;
    std::vector<double> baseLen_;
    std::vector<double> genLen_;
    double genDist_;
    FractalApproxParam param_{};
    size_t ordinal_{};
    bool isLast_{ false };
    bool is_end_{ false };

    Vec2d value_;

    std::vector<GenerationState> state_;
    size_t actualMaxGen_{};
};

using FractalNGenIterator =
    FractalIterator<FractalNGen>;

using FractalApproxIterator =
    FractalIterator<FractalApprox>;

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
