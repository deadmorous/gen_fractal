#pragma once

#include "fractalgenerator.h"

#include <QWidget>

#include <span>

class FractalView : public QWidget
{
    Q_OBJECT
public:
    explicit FractalView(
        FractalGeneratorObject* fractalGenerator,
        QWidget *parent = nullptr);

    auto generations() const noexcept -> int;
    auto antialiasing() const noexcept -> bool;
    auto advancedPen() const noexcept -> bool;
    auto allGenerations() const noexcept -> bool;

public slots:
    auto setGenerations(int generations) -> void;
    auto setAntialiasing(bool enabled) -> void;
    auto setAdvancedPen(bool enabled) -> void;
    auto setAllGenerations(bool enabled) -> void;

protected:
    auto paintEvent(QPaintEvent *event)
        -> void override;

signals:

private:

    auto drawPolyLine(QPainter& painter,
                      std::span<const Vec2d> polyline,
                      QPen pen)
        -> void;

    static auto nextGen(std::span<const Vec2d> base, std::span<const Vec2d> gen)
        -> std::vector<Vec2d>;

    FractalGeneratorObject* fractalGenerator_;
    int generations_{5};
    bool antialiasing_{true};
    bool advancedPen_{true};
    bool allGenerations_{true};
};
