#pragma once

#include "fractalgenerator.h"

#include <QGraphicsScene>
#include <QGraphicsView>

#include <memory>

class FractalGeneratorView : public QGraphicsView
{
    Q_OBJECT
public:
    ~FractalGeneratorView();

    explicit FractalGeneratorView(
        FractalGeneratorObject* fractalGeneratorObject,
        QWidget* parent = nullptr);

public slots:
    auto setSelectedPointCoords(double x, double y) -> void;

signals:
    auto pointSelected(double x, double y) -> void;
    auto pointDeselected() -> void;

protected:
    auto mouseMoveEvent(QMouseEvent* event)
        -> void override;

    auto mousePressEvent(QMouseEvent* event)
        -> void override;

    auto mouseReleaseEvent(QMouseEvent* event)
        -> void override;

private slots:
    auto onFractalGeneratorChanged()
        -> void;

private:

    struct Impl;
    std::unique_ptr<Impl> impl_;
};
