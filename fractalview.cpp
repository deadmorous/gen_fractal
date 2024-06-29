#include "fractalview.h"

#include "bbox2.hpp"
#include "fractal_iter.hpp"
#include "vec2_qt.hpp"

#include <QPainter>
#include <QPainterPath>

#include <chrono>
#include <sstream>

namespace {

auto polylineBbox(std::span<const Vec2d> polyline)
    -> Bbox2d
{
    auto result = Bbox2d{};

    for (const auto& v : polyline)
        result << v;

    return result;
}

auto reportTime(std::ostream& s,
                std::chrono::steady_clock::time_point t1,
                std::chrono::steady_clock::time_point t2,
                std::string_view message)
    -> void
{
    auto msec =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
    s << message << ": " << static_cast<double>(msec) / 1000 << " s\n";
}

struct FractalPolyLineInfo final
{
    size_t vertexCount{};
    size_t maxGen{};
};

template <typename Range>
auto drawPolyLine(QPainter& painter,
                  const Range& polyline,
                  QPen pen)
    -> FractalPolyLineInfo
{
    auto path =
        QPainterPath{};

    auto it = polyline.begin();
    auto end = polyline.end();

    assert(it != end);

    path.moveTo(toQPointF(*it));

    size_t vertexCount = 1;
    for (++it; it!=end; ++it, ++vertexCount)
        path.lineTo(toQPointF(*it));

    pen.setCosmetic(true);
    painter.strokePath(path, pen);

    return { vertexCount, it.impl().actualMaxGen() };
}

template <typename T>
auto setWidgetParam(QWidget* widget, T& dst, const T& src)
    -> void
{
    if (dst == src)
        return;

    dst = src;
    widget->update();
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

    auto fseqApprox = [&](double scale)
    {
        return fractalSeq<FractalApprox>(
            base,
            fg,
            FractalApproxParam{
                .maxGen = approxAlgorithmMaxGen_,
                .maxOrdinal = approxAlgorithmMaxVertexCount_,
                .minLength = 1. / scale
            } );
    };

    size_t scale;
    {
        auto bb = Bbox2d {};
        auto bboxGen =
            approxAlgorithmMaxGen_? approxAlgorithmBboxGen_: generations_;
        for (const auto& v: fseq(bboxGen))
            bb << v;

        auto c_bb = bb.center();
        auto bb_margin = 0.55 * bb.size();
        bb << c_bb - bb_margin << c_bb + bb_margin;

        auto rc = rect();
        auto r_rc = static_cast<double>(rc.width()) / rc.height();
        auto r_bb = bb.size(0) / bb.size(1);
        scale =
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

    if (antialiasing_)
        p.setRenderHint(QPainter::Antialiasing);

    auto polylineInfo = FractalPolyLineInfo{};
    if (approxAlgorithm_)
        polylineInfo = drawPolyLine(p, fseqApprox(scale), QPen{});
    else
    {
        size_t gen = allGenerations_? 0: generations_;
        for (; gen<=generations_; ++gen)
        {
            auto pen = QPen{};
            if (fancyPen_)
            {
                auto width = double(1 << (generations_-gen));
                auto hue = static_cast<double>(gen) / (generations_+1);
                auto alpha = static_cast<double>(gen+1) / (generations_+1);
                auto color = QColor::fromHsvF(hue, 0.8, 0.8, alpha);
                pen = QPen{ color, width };
            }
            polylineInfo = drawPolyLine(p, fseq(gen), pen);
        }
    }
    auto time_2 = clock::now();

    std::ostringstream status;
    reportTime(status, time_0, time_1, "Computing bounding box");
    reportTime(status, time_1, time_2, "Drawing fractal");
    status << "Vertices: " << polylineInfo.vertexCount << std::endl;
    status << "Max. generation: " << polylineInfo.maxGen << std::endl;
    status << "Scale: " << scale << std::endl;

    emit renderingStatus(QString::fromStdString(status.str()));
}

auto FractalView::generations() const noexcept
    -> size_t
{ return generations_; }

auto FractalView::antialiasing() const noexcept
    -> bool
{ return antialiasing_; }

auto FractalView::fancyPen() const noexcept
    -> bool
{ return fancyPen_; }

auto FractalView::allGenerations() const noexcept
    -> bool
{ return allGenerations_; }

auto FractalView::approxAlgorithm() const noexcept
    -> bool
{ return approxAlgorithm_; }

auto FractalView::approxAlgorithmBboxGen() const noexcept
    -> size_t
{ return approxAlgorithmBboxGen_; }

auto FractalView::approxAlgorithmMaxGen() const noexcept
    -> size_t
{ return approxAlgorithmMaxGen_; }

auto FractalView::approxAlgorithmMaxVertexCount() const noexcept
    -> size_t
{ return approxAlgorithmMaxVertexCount_; }

auto FractalView::setGenerations(size_t generations)
    -> void
{ setWidgetParam(this, generations_, generations); }

auto FractalView::setAntialiasing(bool enabled)
    -> void
{ setWidgetParam(this, antialiasing_, enabled); }

auto FractalView::setFancyPen(bool enabled)
    -> void
{ setWidgetParam(this, fancyPen_, enabled); }

auto FractalView::setAllGenerations(bool enabled)
    -> void
{ setWidgetParam(this, allGenerations_, enabled); }

auto FractalView::setApproxAlgorithm(bool enabled)
    -> void
{ setWidgetParam(this, approxAlgorithm_, enabled); }

auto FractalView::setApproxAlgorithmBboxGen(size_t bboxGen)
    -> void
{ setWidgetParam(this, approxAlgorithmBboxGen_, bboxGen); }

auto FractalView::setApproxAlgorithmMaxGen(size_t maxGen)
    -> void
{ setWidgetParam(this, approxAlgorithmMaxGen_, maxGen); }

auto FractalView::setApproxAlgorithmMaxVertexCount(size_t maxVertexCount)
    -> void
{ setWidgetParam(this, approxAlgorithmMaxVertexCount_, maxVertexCount); }
