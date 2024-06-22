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
};
