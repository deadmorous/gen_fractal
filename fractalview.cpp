#include "fractalview.h"

#include "bbox2.hpp"
#include "vec2_qt.hpp"

#include <QPainter>
#include <QPainterPath>

#include <chrono>
#include <numbers>

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
    // auto msec =
    //     std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
    // qDebug() << message << ": " << static_cast<double>(msec) / 1000 << " s";
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

    auto fs = std::vector<std::vector<Vec2d>>{};
    fs.push_back({ {0., 0.}, {1., 0.} });
    for (size_t gen=0; gen<generations_; ++gen)
        fs.push_back(nextGen(fs.back(), fg));

    {
        auto bb = Bbox2d {};
        for (const auto& f: fs)
            bb << polylineBbox(f);

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
        const auto& f = fs[gen];
        auto pen = QPen{};
        if (advancedPen_)
        {
            auto width = double(1 << (generations_-gen));
            auto hue = static_cast<double>(gen) / (generations_+1);
            auto alpha = static_cast<double>(gen+1) / (generations_+1);
            auto color = QColor::fromHsvF(hue, 0.8, 0.8, alpha);
            pen = QPen{ color, width };
        }
        drawPolyLine(p, f, pen);
    }
    auto time_2 = clock::now();
    reportTime(time_1, time_2, "Drawing fractal");
}

auto FractalView::drawPolyLine(QPainter& painter,
                               std::span<const Vec2d> polyline,
                               QPen pen)
    -> void
{
    auto path =
        QPainterPath{};

    path.moveTo(toQPointF(polyline[0]));
    for (size_t index=1, n=polyline.size(); index<n; ++index)
        path.lineTo(toQPointF(polyline[index]));

    pen.setCosmetic(true);
    painter.strokePath(path, pen);
}

auto FractalView::nextGen(std::span<const Vec2d> base,
                          std::span<const Vec2d> gen)
    -> std::vector<Vec2d>
{
    Q_ASSERT(base.size() > 1);
    Q_ASSERT(gen.size() > 1);

    auto nbase = base.size() - 1;
    auto ngen = gen.size() - 1;

    auto result = std::vector<Vec2d>{};
    result.reserve(nbase * ngen + 1);

    result.push_back(base[0]);

    const auto& g1 = gen.front();
    auto dr_gen = gen.back() - g1;
    auto length_gen = dr_gen.norm();
    auto angle_gen = atan2(dr_gen[1], dr_gen[0]);

    for (size_t ibase=0; ibase<nbase; ++ibase)
    {
        auto b1 = base[ibase];
        auto b2 = base[ibase+1];
        auto dr_base = b2 - b1;
        auto length_base = dr_base.norm();
        auto angle_base = atan2(dr_base[1], dr_base[0]);
        auto scale = length_base / length_gen;
        auto angle = angle_base - angle_gen;
        auto t = QTransform{};
        t
            .translate(b1[0], b1[1])
            .scale(scale, scale)
            .rotate(angle / std::numbers::pi_v<double> * 180)
            .translate(-g1[0], -g1[1]);

        for (size_t igen=1; igen<=ngen; ++igen)
        {
            auto r = t.map(toQPointF(gen[igen]));
            result.push_back(toVec2d(r));
        }
    }

    return result;
}

auto FractalView::generations() const noexcept
    -> int
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

auto FractalView::setGenerations(int generations)
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
