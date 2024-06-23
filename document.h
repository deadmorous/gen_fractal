#pragma once

#include "fractalgenerator.h"

#include <QObject>

class Document : public QObject
{
    Q_OBJECT
public:
    explicit Document(QObject *parent = nullptr);

    auto fractalGenerator() -> FractalGeneratorObject*;

    auto fileName() const -> QString;

public slots:
    auto newDocument() -> void;

    auto open(const QString& fileName) -> void;
    auto saveAs(const QString& fileName) -> void;

    auto open() -> void;
    auto save() -> void;
    auto saveAs() -> void;

signals:

private:
    FractalGeneratorObject fractalGenerator_;
    QString fileName_;
};
