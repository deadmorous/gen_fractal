#include "mainwindow.h"

#include "fractalview.h"
#include "fractalgenerator.h"
#include "fractalgeneratorview.h"

#include <QGraphicsView>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setMinimumSize(QSize{800, 600});

    auto fractalGeneratorObject = new FractalGeneratorObject(this);

    fractalGeneratorObject->setFractalGenerator(
        { {0., 0.}, {100., 0.} } );

    auto fractalGeneratorView =
        new FractalGeneratorView(fractalGeneratorObject);

    // auto sceneView = new QGraphicsView(scene);
    auto splitter = new QSplitter;

    splitter->addWidget(fractalGeneratorView);
    splitter->addWidget(new FractalView{ fractalGeneratorObject });

    setCentralWidget(splitter);
}

MainWindow::~MainWindow() {}
