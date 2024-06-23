#include "mainwindow.h"

#include "controlsdialog.h"
#include "document.h"
#include "fractalview.h"
#include "fractalgeneratorview.h"

#include <QGraphicsView>
#include <QMenuBar>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setMinimumSize(QSize{800, 600});

    auto doc = new Document(this);

    auto fractalGeneratorView =
        new FractalGeneratorView(doc->fractalGenerator());

    auto mainSplitter = new QSplitter;

    auto toolSplitter = new QSplitter(Qt::Vertical);
    toolSplitter->addWidget(fractalGeneratorView);

    auto controlsDialog = new ControlsDialog;
    toolSplitter->addWidget(controlsDialog);

    mainSplitter->addWidget(toolSplitter);

    auto fractalView = new FractalView{ doc->fractalGenerator() };
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

    auto* menu = new QMenuBar;

    auto* fileMenu = menu->addMenu("&File");

    auto* newAction = fileMenu->addAction("&New", QKeySequence::New);
    connect(newAction, &QAction::triggered, doc, &Document::newDocument);

    auto* openAction = fileMenu->addAction("&Open...", QKeySequence::Open);
    connect(openAction, &QAction::triggered, doc, qOverload<>(&Document::open));

    auto* saveAction = fileMenu->addAction("&Save", QKeySequence::Save);
    connect(saveAction, &QAction::triggered, doc, qOverload<>(&Document::save));

    auto* saveAsAction =
        fileMenu->addAction("Save &As...", QKeySequence::SaveAs);
    connect(
        saveAsAction, &QAction::triggered, doc, qOverload<>(&Document::saveAs));

    fileMenu->addSeparator();
    auto* quitAction = fileMenu->addAction("&Quit", QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    setMenuBar(menu);
}

MainWindow::~MainWindow() {}
