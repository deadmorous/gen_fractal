#include "fractalview.h"

#include "bbox2.hpp"
#include "fractal_iter.hpp"
#include "vec2_qt.hpp"

#include <QPainter>
#include <QPainterPath>

#include <chrono>

namespace {

auto polylineBbox(std::span<const Vec2d> polyline)
    -> Bbox2d
{
    auto result = Bbox2d{};

    for (const auto& v : polyline)
        result << v;

    return result;
}

auto reportTime(std::chrono::steady_clock::time_point t1,
                std::chrono::steady_clock::time_point t2,
                std::string_view message)
    -> void
{
    auto msec =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
    qDebug() << message << ": " << static_cast<double>(msec) / 1000 << " s";
}

template <typename Range>
auto drawPolyLine(QPainter& painter,
                  const Range& polyline,
                  QPen pen)
    -> void
{
    auto path =
        QPainterPath{};

    auto it = polyline.begin();
    auto end = polyline.end();

    assert(it != end);

    path.moveTo(toQPointF(*it));

    for (++it; it!=end; ++it)
        path.lineTo(toQPointF(*it));

    pen.setCosmetic(true);
    painter.strokePath(path, pen);
}

} // anonymous namespace



FractalView::FractalView(FractalGeneratorObject* fractalGenerator,
                         QWidget *parent):
    QWidget{parent},
    fractalGenerator_{ fractalGenerator }
{
    connect(
        fractalGenerator_,
        &FractalGeneratorObject::fractalGeneratorChanged,
        this,
        qOverload<>(&FractalView::update));
}

auto FractalView::paintEvent(QPaintEvent *event)
    -> void
{
    auto p = QPainter{this};
    p.fillRect(rect(), Qt::white);

    const auto& fg = fractalGenerator_->fractalGenerator();
    if (fg.size() < 2)
        return;

    using clock = std::chrono::steady_clock;
    auto time_0 = clock::now();

    auto base = std::vector<Vec2d>{ {0., 0.}, {1., 0.} };

    auto fseq = [&](size_t gen)
    { return fractalSeq<FractalNGen>( base, fg, gen ); };

    {
        auto bb = Bbox2d {};
        for (const auto& v: fseq(generations_))
            bb << v;

        auto c_bb = bb.center();
        auto bb_margin = 0.55 * bb.size();
        bb << c_bb - bb_margin << c_bb + bb_margin;

        auto rc = rect();
        auto r_rc = static_cast<double>(rc.width()) / rc.height();
        auto r_bb = bb.size(0) / bb.size(1);
        auto scale =
            r_bb > r_rc?  rc.width() / bb.size(0) : rc.height() / bb.size(1);
        auto c_rc = toVec2d(rc.center());

        auto t = QTransform{};
        t
            .translate(c_rc[0], c_rc[1])
            .scale(scale, scale)
            .translate(-c_bb[0], -c_bb[1]);
        p.setTransform(t);
    }
    auto time_1 = clock::now();
    reportTime(time_0, time_1, "Computing fractal");

    if (antialiasing_)
        p.setRenderHint(QPainter::Antialiasing);

    size_t gen = allGenerations_? 0: generations_;
    for (; gen<=generations_; ++gen)
    {
        auto pen = QPen{};
        if (advancedPen_)
        {
            auto width = double(1 << (generations_-gen));
            auto hue = static_cast<double>(gen) / (generations_+1);
            auto alpha = static_cast<double>(gen+1) / (generations_+1);
            auto color = QColor::fromHsvF(hue, 0.8, 0.8, alpha);
            pen = QPen{ color, width };
        }
        drawPolyLine(p, fseq(gen), pen);
    }
    auto time_2 = clock::now();
    reportTime(time_1, time_2, "Drawing fractal");
}

auto FractalView::generations() const noexcept
    -> size_t
{ return generations_; }

auto FractalView::antialiasing() const noexcept
    -> bool
{ return antialiasing_; }

auto FractalView::advancedPen() const noexcept
    -> bool
{ return advancedPen_; }

auto FractalView::allGenerations() const noexcept
    -> bool
{ return allGenerations_; }

auto FractalView::setGenerations(size_t generations)
    -> void
{
    if (generations_ == generations)
        return;

    generations_ = generations;
    update();
}

auto FractalView::setAntialiasing(bool enabled)
    -> void
{
    if (antialiasing_ == enabled)
        return;

    antialiasing_ = enabled;
    update();
}

auto FractalView::setAdvancedPen(bool enabled) -> void
{
    if (advancedPen_ == enabled)
        return;

    advancedPen_ = enabled;
    update();
}

auto FractalView::setAllGenerations(bool enabled) -> void
{
    if (allGenerations_ == enabled)
        return;

    allGenerations_ = enabled;
    update();
}
