#pragma once

#include "fractalgenerator.h"
#include "fractalview_param.h"

#include <QWidget>

class FractalView : public QWidget
{
    Q_OBJECT
public:
    explicit FractalView(
        FractalGeneratorObject* fractalGenerator,
        QWidget *parent = nullptr);

    auto generations() const noexcept -> size_t;
    auto antialiasing() const noexcept -> bool;
    auto fancyPen() const noexcept -> bool;
    auto allGenerations() const noexcept -> bool;
    auto approxAlgorithm() const noexcept -> bool;
    auto approxAlgorithmBboxGen() const noexcept -> size_t;
    auto approxAlgorithmMaxGen() const noexcept -> size_t;
    auto approxAlgorithmMaxVertexCount() const noexcept -> size_t;
    auto adjustScale() const noexcept -> double;

    auto param() const noexcept -> const FractalViewParam&;

public slots:
    auto setGenerations(size_t generations) -> void;
    auto setAntialiasing(bool enabled) -> void;
    auto setFancyPen(bool enabled) -> void;
    auto setAllGenerations(bool enabled) -> void;
    auto setApproxAlgorithm(bool enabled) -> void;
    auto setApproxAlgorithmBboxGen(size_t bboxGen) -> void;
    auto setApproxAlgorithmMaxGen(size_t maxGen) -> void;
    auto setApproxAlgorithmMaxVertexCount(size_t maxVertexCount) -> void;
    auto setAdjustScale(double adjustScale) -> void;

    auto setParam(const FractalViewParam&) -> void;

protected:
    auto paintEvent(QPaintEvent *event)
        -> void override;

signals:
    auto renderingStatus(const QString&)
        -> void;

private:

    FractalGeneratorObject* fractalGenerator_;
    FractalViewParam param_;
};
