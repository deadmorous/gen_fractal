#include "render_fractal.hpp"

#include "bbox2.hpp"
#include "fractal_iter.hpp"
#include "vec2_qt.hpp"

#include <QPainter>
#include <QPainterPath>

#include <chrono>

namespace {

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

} // anonymous namespace



auto renderFractal(QPainter& p,
                   const QRect& rect,
                   std::span<const Vec2d> base,
                   std::span<const Vec2d> gen,
                   const FractalViewParam& param)
    -> RenderFractlalResult
{
    p.fillRect(rect, Qt::white);

    if (gen.size() < 2)
        return {};

    using clock = std::chrono::steady_clock;
    auto time_0 = clock::now();

    auto fseq = [&](size_t maxGen)
    { return fractalSeq<FractalNGen>(base, gen, maxGen); };

    auto fseqApprox = [&](double scale)
    {
        return fractalSeq<FractalApprox>(
            base,
            gen,
            FractalApproxParam{
                .maxGen = param.approxAlgorithmMaxGen,
                .maxOrdinal = param.approxAlgorithmMaxVertexCount,
                .minLength = 1. / scale
            } );
    };

    double scale;
    {
        auto bb = Bbox2d {};
        auto bboxGen =
            param.approxAlgorithmMaxGen
                ? param.approxAlgorithmBboxGen
                : param.generations;
        for (const auto& v: fseq(bboxGen))
            bb << v;

        auto c_bb = bb.center();
        auto bb_margin = 0.55 * bb.size();
        bb << c_bb - bb_margin << c_bb + bb_margin;

        auto r_rc = static_cast<double>(rect.width()) / rect.height();
        auto r_bb = bb.size(0) / bb.size(1);
        scale = r_bb > r_rc
            ?  rect.width() / bb.size(0)
            : rect.height() / bb.size(1);
        scale /= param.adjustScale;
        auto c_rc = toVec2d(rect.center());

        auto t = QTransform{};
        t
            .translate(c_rc[0], c_rc[1])
            .scale(scale, scale)
            .translate(-c_bb[0], -c_bb[1]);
        p.setTransform(t);
    }
    auto time_1 = clock::now();

    if (param.antialiasing)
        p.setRenderHint(QPainter::Antialiasing);

    auto polylineInfo = FractalPolyLineInfo{};
    if (param.approxAlgorithm)
        polylineInfo = drawPolyLine(p, fseqApprox(scale), QPen{});
    else
    {
        size_t gen = param.allGenerations? 0: param.generations;
        for (; gen<=param.generations; ++gen)
        {
            auto pen = QPen{};
            if (param.fancyPen)
            {
                auto width = double(1 << (param.generations-gen));
                auto hue = static_cast<double>(gen) / (param.generations+1);
                auto alpha = static_cast<double>(gen+1) / (param.generations+1);
                auto color = QColor::fromHsvF(hue, 0.8, 0.8, alpha);
                pen = QPen{ color, width };
            }
            polylineInfo = drawPolyLine(p, fseq(gen), pen);
        }
    }
    auto time_2 = clock::now();

    return {
        .computeBbTime = time_1 - time_0,
        .renderTime = time_2 - time_1,
        .vertexCount = polylineInfo.vertexCount,
        .maxGen = polylineInfo.maxGen,
        .scale = scale
    };
}
