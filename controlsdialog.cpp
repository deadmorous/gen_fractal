#include "controlsdialog.h"
#include "ui_controlsdialog.h"

#include "scoped_true.hpp"

ControlsDialog::ControlsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ControlsDialog)
{ ui->setupUi(this); }

ControlsDialog::~ControlsDialog()
{
    delete ui;
}

void ControlsDialog::on_generations_valueChanged(int arg1)
{ emit generationsEdited(arg1); }

void ControlsDialog::on_edit_x_valueChanged(double arg1)
{ emitPointCoordsEdited(); }


void ControlsDialog::on_edit_y_valueChanged(double arg1)
{ emitPointCoordsEdited(); }

void ControlsDialog::setPointCoords(double x, double y)
{
    if (settingPointCoords_)
        return;

    auto scopedTrue = ScopedTrue{settingPointCoords_};

    ui->pointCoordsWidget->setEnabled(true);
    ui->edit_x->setValue(x);
    ui->edit_y->setValue(y);
}

void ControlsDialog::disablePoint()
{
    auto scopedTrue = ScopedTrue{settingPointCoords_};

    ui->pointCoordsWidget->setEnabled(false);
    ui->edit_x->setValue(0);
    ui->edit_y->setValue(0);
}

void ControlsDialog::setGenerations(size_t generations)
{ ui->generations->setValue(generations); }

void ControlsDialog::setAntialiasing(bool enabled)
{ ui->checkAntialiasing->setChecked(enabled); }

void ControlsDialog::setFancyPen(bool enabled)
{ ui->checkPen->setChecked(enabled); }

void ControlsDialog::setAllGenerations(bool enabled)
{ ui->checkAllGen->setChecked(enabled); }

void ControlsDialog::setApproxAlgorithm(bool enabled)
{ ui->checkApprox->setChecked(enabled); }

void ControlsDialog::setApproxBboxGen(size_t bboxGen)
{ ui->spinApproxBboxGen->setValue(bboxGen); }

void ControlsDialog::setApproxMaxGen(size_t maxGen)
{ ui->spinApproxMaxGen->setValue(maxGen); }

void ControlsDialog::setApproxMaxVertices(size_t maxVertices)
{ ui->spinApproxMaxVertices->setValue(maxVertices); }

void ControlsDialog::setAdjustScale(double adjustScale)
{ ui->spinAdjustScale->setValue(adjustScale); }

void ControlsDialog::emitPointCoordsEdited()
{
    if (settingPointCoords_)
        return;

    auto scopedTrue = ScopedTrue{settingPointCoords_};

    auto x = ui->edit_x->text().toDouble();
    auto y = ui->edit_y->text().toDouble();
    emit pointCoordsEdited(x, y);
}

void ControlsDialog::on_checkAntialiasing_stateChanged(int arg1)
{ emit antialiasingChanged(arg1 == Qt::Checked); }

void ControlsDialog::on_checkPen_stateChanged(int arg1)
{ emit fancyPenChanged(arg1 == Qt::Checked); }

void ControlsDialog::on_checkAllGen_stateChanged(int arg1)
{ emit allGenerationsChanged(arg1 == Qt::Checked); }

void ControlsDialog::on_checkApprox_stateChanged(int arg1)
{ emit approxAlgorithmChanged(arg1 == Qt::Checked); }

void ControlsDialog::on_spinApproxBboxGen_valueChanged(int arg1)
{ emit approxBboxGenEdited(arg1); }

void ControlsDialog::on_spinApproxMaxGen_valueChanged(int arg1)
{ emit approxMaxGenEdited(arg1); }

void ControlsDialog::on_spinApproxMaxVertices_valueChanged(int arg1)
{ emit approxMaxVerticesEdited(arg1); }

void ControlsDialog::on_spinAdjustScale_valueChanged(double arg1)
{ emit adjustScaleEdited(arg1); }

