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
    FractalGeneratorObject(QObject* parent = nullptr):
        QObject{ parent },
        fractalGenerator_{ {0., 0.}, {100., 0.} }
    {}

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
