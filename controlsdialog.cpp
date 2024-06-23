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

void ControlsDialog::setGenerations(int generations)
{ ui->generations->setValue(generations); }

void ControlsDialog::setAntialiasing(bool enabled)
{ ui->checkAntialiasing->setChecked(enabled); }

void ControlsDialog::setAdvancedPen(bool enabled)
{ ui->checkPen->setChecked(enabled); }

void ControlsDialog::setAllGenerations(bool enabled)
{ ui->checkAllGen->setChecked(enabled); }

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
{ emit advancedPenChanged(arg1 == Qt::Checked); }

void ControlsDialog::on_checkAllGen_stateChanged(int arg1)
{ emit allGenerationsChanged(arg1 == Qt::Checked); }
