#include "mainwindow.h"

#include "controlsdialog.h"
#include "fractalview.h"
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

    auto mainSplitter = new QSplitter;

    auto toolSplitter = new QSplitter(Qt::Vertical);
    toolSplitter->addWidget(fractalGeneratorView);

    auto controlsDialog = new ControlsDialog;
    toolSplitter->addWidget(controlsDialog);

    mainSplitter->addWidget(toolSplitter);

    auto fractalView = new FractalView{ fractalGeneratorObject };
    mainSplitter->addWidget(fractalView);

    controlsDialog->setGenerations(fractalView->generations());
    connect(
        controlsDialog,
        &ControlsDialog::generationsEdited,
        fractalView,
        &FractalView::setGenerations);

    controlsDialog->setAntialiasing(fractalView->antialiasing());
    connect(
        controlsDialog,
        &ControlsDialog::antialiasingChanged,
        fractalView,
        &FractalView::setAntialiasing);

    controlsDialog->setAdvancedPen(fractalView->advancedPen());
    connect(
        controlsDialog,
        &ControlsDialog::advancedPenChanged,
        fractalView,
        &FractalView::setAdvancedPen);

    controlsDialog->setAllGenerations(fractalView->allGenerations());
    connect(
        controlsDialog,
        &ControlsDialog::allGenerationsChanged,
        fractalView,
        &FractalView::setAllGenerations);

    controlsDialog->disablePoint();
    connect(
        controlsDialog,
        &ControlsDialog::pointCoordsEdited,
        fractalGeneratorView,
        &FractalGeneratorView::setSelectedPointCoords);
    connect(
        fractalGeneratorView,
        &FractalGeneratorView::pointDeselected,
        controlsDialog,
        &ControlsDialog::disablePoint);
    connect(
        fractalGeneratorView,
        &FractalGeneratorView::pointSelected,
        controlsDialog,
        &ControlsDialog::setPointCoords);

    setCentralWidget(mainSplitter);
}

MainWindow::~MainWindow() {}
