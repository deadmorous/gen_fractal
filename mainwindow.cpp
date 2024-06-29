#include "mainwindow.h"

#include "controlsdialog.h"
#include "document.h"
#include "fractalview.h"
#include "fractalgeneratorview.h"

#include <QGraphicsView>
#include <QLabel>
#include <QMenuBar>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , doc_{ new Document(this) }
{
    setMinimumSize(QSize{800, 600});

    auto fractalGeneratorView =
        new FractalGeneratorView(doc_->fractalGenerator());

    auto mainSplitter = new QSplitter;

    auto toolSplitter = new QSplitter(Qt::Vertical);
    toolSplitter->addWidget(fractalGeneratorView);

    auto controlsDialog = new ControlsDialog;
    toolSplitter->addWidget(controlsDialog);

    auto statusWidget = new QLabel(tr("Rendering status"));
    statusWidget->setMinimumHeight(200);
    toolSplitter->addWidget(statusWidget);

    mainSplitter->addWidget(toolSplitter);

    auto fractalView = new FractalView{ doc_->fractalGenerator() };
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

    controlsDialog->setFancyPen(fractalView->fancyPen());
    connect(
        controlsDialog,
        &ControlsDialog::fancyPenChanged,
        fractalView,
        &FractalView::setFancyPen);

    controlsDialog->setAllGenerations(fractalView->allGenerations());
    connect(
        controlsDialog,
        &ControlsDialog::allGenerationsChanged,
        fractalView,
        &FractalView::setAllGenerations);

    controlsDialog->setApproxAlgorithm(fractalView->approxAlgorithm());
    connect(
        controlsDialog,
        &ControlsDialog::approxAlgorithmChanged,
        fractalView,
        &FractalView::setApproxAlgorithm);

    controlsDialog->setApproxBboxGen(fractalView->approxAlgorithmBboxGen());
    connect(
        controlsDialog,
        &ControlsDialog::approxBboxGenEdited,
        fractalView,
        &FractalView::setApproxAlgorithmBboxGen);

    controlsDialog->setApproxMaxGen(fractalView->approxAlgorithmMaxGen());
    connect(
        controlsDialog,
        &ControlsDialog::approxMaxGenEdited,
        fractalView,
        &FractalView::setApproxAlgorithmMaxGen);

    controlsDialog->setApproxMaxVertices(
        fractalView->approxAlgorithmMaxVertexCount());
    connect(
        controlsDialog,
        &ControlsDialog::approxMaxVerticesEdited,
        fractalView,
        &FractalView::setApproxAlgorithmMaxVertexCount);

    controlsDialog->setAdjustScale(fractalView->adjustScale());
    connect(
        controlsDialog,
        &ControlsDialog::adjustScaleEdited,
        fractalView,
        &FractalView::setAdjustScale);

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

    connect(
        fractalView,
        &FractalView::renderingStatus,
        statusWidget,
        &QLabel::setText);

    setCentralWidget(mainSplitter);

    auto* menu = new QMenuBar;

    auto* fileMenu = menu->addMenu("&File");

    auto* newAction = fileMenu->addAction("&New", QKeySequence::New);
    connect(newAction, &QAction::triggered, doc_, &Document::newDocument);

    auto* openAction = fileMenu->addAction("&Open...", QKeySequence::Open);
    connect(openAction, &QAction::triggered,
            doc_, qOverload<>(&Document::open));

    auto* saveAction = fileMenu->addAction("&Save", QKeySequence::Save);
    connect(saveAction, &QAction::triggered,
            doc_, qOverload<>(&Document::save));

    auto* saveAsAction =
        fileMenu->addAction("Save &As...", QKeySequence::SaveAs);
    connect(
        saveAsAction, &QAction::triggered,
        doc_, qOverload<>(&Document::saveAs));

    fileMenu->addSeparator();

    auto* logStateAction = fileMenu->addAction(
        "&Log state", QKeyCombination(Qt::CTRL, Qt::Key_P));
    connect(logStateAction, &QAction::triggered,
            fractalView, &FractalView::logState);

    fileMenu->addSeparator();

    auto* quitAction = fileMenu->addAction("&Quit", QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    setMenuBar(menu);
}

auto MainWindow::open(const QString& fileName)
    -> void
{ doc_->open(fileName); }
