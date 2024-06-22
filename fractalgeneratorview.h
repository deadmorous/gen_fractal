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

protected:
    auto mouseMoveEvent(QMouseEvent* event)
        -> void override;

    auto mousePressEvent(QMouseEvent* event)
        -> void override;

private:

    struct Impl;
    std::unique_ptr<Impl> impl_;
};
