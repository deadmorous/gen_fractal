#pragma once

#include "fractalgenerator.h"

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
    auto advancedPen() const noexcept -> bool;
    auto allGenerations() const noexcept -> bool;

public slots:
    auto setGenerations(size_t generations) -> void;
    auto setAntialiasing(bool enabled) -> void;
    auto setAdvancedPen(bool enabled) -> void;
    auto setAllGenerations(bool enabled) -> void;

protected:
    auto paintEvent(QPaintEvent *event)
        -> void override;

signals:

private:

    FractalGeneratorObject* fractalGenerator_;
    size_t generations_{5};
    bool antialiasing_{true};
    bool advancedPen_{true};
    bool allGenerations_{true};
};
