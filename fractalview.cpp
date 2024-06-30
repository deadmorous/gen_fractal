#include "fractalview.h"

#include "anim_param.hpp"
#include "render_fractal.hpp"

#include <QMessageBox>
#include <QPainter>

#include <filesystem>
#include <sstream>

using namespace std::string_literals;

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

auto FractalView::logState() -> void
{
    namespace fs = std::filesystem;
    static const auto fileName = "gen_fractal.log"s;

    if (!log_.is_open())
    {
        auto filePath =
            fs::absolute(fileName);

        if (fs::exists(fileName))
        {
            std::ostringstream s;
            s << "Log file " << filePath << " exists. Overwrite?";
            auto result =
                QMessageBox::question(
                this, QString(), QString::fromStdString(s.str()));
            if (result != QMessageBox::Yes)
                return;
        }

        log_.open(filePath);
        if (log_.is_open())
        {
            std::ostringstream s;
            s << "Logging this and any further states to file "
              << filePath;
            QMessageBox::information(
                this, QString(),
                QString::fromStdString(s.str()));
        }

        else
        {
            std::ostringstream s;
            s << "Unable to open output file "
              << filePath;
            QMessageBox::critical(
                this, QString(),
                QString::fromStdString(s.str()));
            return;
        }

        log_ << "base_x#,base_y#...,*,gen_x#,gen_y#...,*";
        for (auto name : field_names_of(Type<FractalViewParam>))
            log_ << ',' << name;
        log_ << ",width,height";
        for (auto name : field_names_of(Type<AnimParam>))
            log_ << ',' << name;
        log_ << std::endl;
    }

    auto log2dline = [&](std::span<const Vec2d> line)
    {
        for (auto& v: line)
            log_ << v[0] << ',' << v[1] << ',';
        log_ << '*';
    };

    auto base = std::vector<Vec2d>{ {0., 0.}, {1., 0.} };
    log2dline( base );
    log_ << ',';
    log2dline( fractalGenerator_->fractalGenerator() );

    auto logStruct = [&](const auto& x)
    {
        auto fields = fields_of(x);
        [&]<size_t... I>(std::index_sequence<I...>)
        {
            ((log_ << ',' << std::get<I>(fields)), ...);
        }(std::make_index_sequence<std::tuple_size_v<decltype(fields)>>());
    };

    logStruct(param_);

    auto rc = rect();
    log_ << ',' << rc.width() << ',' << rc.height();

    logStruct(AnimParam{});

    log_ << std::endl;
}
