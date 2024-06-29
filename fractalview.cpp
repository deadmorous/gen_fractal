#include "fractalview.h"

#include "render_fractal.hpp"

// #include "bbox2.hpp"
// #include "fractal_iter.hpp"
// #include "vec2_qt.hpp"

#include <QPainter>
#include <QPainterPath>

#include <chrono>
#include <sstream>

namespace {

auto reportTime(std::ostream& s,
                std::chrono::nanoseconds dt,
                std::string_view message)
    -> void
{
    auto msec =
        std::chrono::duration_cast<std::chrono::milliseconds>(dt).count();
    s << message << ": " << static_cast<double>(msec) / 1000 << " s\n";
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

    const auto& fg = fractalGenerator_->fractalGenerator();
    auto base = std::vector<Vec2d>{ {0., 0.}, {1., 0.} };
    auto renderResult = renderFractal(p, rect(), base, fg, param_);

    std::ostringstream status;
    reportTime(status, renderResult.computeBbTime, "Computing bounding box");
    reportTime(status, renderResult.renderTime, "Rendering fractal");
    status << "Vertices: " << renderResult.vertexCount << std::endl;
    status << "Max. generation: " << renderResult.maxGen << std::endl;
    status << "Scale: " << renderResult.scale << std::endl;

    emit renderingStatus(QString::fromStdString(status.str()));
}

auto FractalView::generations() const noexcept
    -> size_t
{ return param_.generations; }

auto FractalView::antialiasing() const noexcept
    -> bool
{ return param_.antialiasing; }

auto FractalView::fancyPen() const noexcept
    -> bool
{ return param_.fancyPen; }

auto FractalView::allGenerations() const noexcept
    -> bool
{ return param_.allGenerations; }

auto FractalView::approxAlgorithm() const noexcept
    -> bool
{ return param_.approxAlgorithm; }

auto FractalView::approxAlgorithmBboxGen() const noexcept
    -> size_t
{ return param_.approxAlgorithmBboxGen; }

auto FractalView::approxAlgorithmMaxGen() const noexcept
    -> size_t
{ return param_.approxAlgorithmMaxGen; }

auto FractalView::approxAlgorithmMaxVertexCount() const noexcept
    -> size_t
{ return param_.approxAlgorithmMaxVertexCount; }

auto FractalView::adjustScale() const noexcept -> double
{ return param_.adjustScale; }

auto FractalView::param() const noexcept
    -> const FractalViewParam&
{ return param_; }



auto FractalView::setGenerations(size_t generations)
    -> void
{ setWidgetParam(this, param_.generations, generations); }

auto FractalView::setAntialiasing(bool enabled)
    -> void
{ setWidgetParam(this, param_.antialiasing, enabled); }

auto FractalView::setFancyPen(bool enabled)
    -> void
{ setWidgetParam(this, param_.fancyPen, enabled); }

auto FractalView::setAllGenerations(bool enabled)
    -> void
{ setWidgetParam(this, param_.allGenerations, enabled); }

auto FractalView::setApproxAlgorithm(bool enabled)
    -> void
{ setWidgetParam(this, param_.approxAlgorithm, enabled); }

auto FractalView::setApproxAlgorithmBboxGen(size_t bboxGen)
    -> void
{ setWidgetParam(this, param_.approxAlgorithmBboxGen, bboxGen); }

auto FractalView::setApproxAlgorithmMaxGen(size_t maxGen)
    -> void
{ setWidgetParam(this, param_.approxAlgorithmMaxGen, maxGen); }

auto FractalView::setApproxAlgorithmMaxVertexCount(size_t maxVertexCount)
    -> void
{ setWidgetParam(this, param_.approxAlgorithmMaxVertexCount, maxVertexCount); }

auto FractalView::setAdjustScale(double adjustScale)
    -> void
{ setWidgetParam(this, param_.adjustScale, adjustScale); }

auto FractalView::setParam(const FractalViewParam& param)
    -> void
{
    param_ = param;
    update();
}
