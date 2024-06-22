#pragma once

#include "vec2.hpp"

#include <QObject>

#include <vector>

using FractalGenerator =
    std::vector<Vec2d>;

class FractalGeneratorObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(
        FractalGenerator fractalGenerator
        READ fractalGenerator
        WRITE setFractalGenerator
        NOTIFY fractalGeneratorChanged
        FINAL)

public:
    using QObject::QObject;

    auto fractalGenerator() const
        -> const FractalGenerator&
    { return fractalGenerator_; }

public slots:
    auto setFractalGenerator(FractalGenerator fractalGenerator)
        -> void
    {
        fractalGenerator_ = std::move(fractalGenerator);
        emit fractalGeneratorChanged(fractalGenerator_);
    }

signals:
    auto fractalGeneratorChanged(const FractalGenerator&)
        -> void;

private:

    FractalGenerator fractalGenerator_;
};
