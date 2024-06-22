#ifndef CONTROLSDIALOG_H
#define CONTROLSDIALOG_H

#include <QDialog>

namespace Ui {
class ControlsDialog;
}

class ControlsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ControlsDialog(QWidget *parent = nullptr);
    ~ControlsDialog();

signals:
    void pointCoordsEdited(double x, double y);
    void generationsEdited(int generations);
    void antialiasingChanged(bool enabled);
    void advancedPenChanged(bool enabled);
    void allGenerationsChanged(bool enabled);

public slots:
    void setPointCoords(double x, double y);
    void disablePoint();
    void setGenerations(int generations);
    void setAntialiasing(bool enabled);
    void setAdvancedPen(bool enabled);
    void setAllGenerations(bool enabled);

private slots:
    void on_generations_valueChanged(int arg1);
    void on_edit_x_valueChanged(double arg1);
    void on_edit_y_valueChanged(double arg1);

    void on_checkAntialiasing_stateChanged(int arg1);

    void on_checkPen_stateChanged(int arg1);

    void on_checkAllGen_stateChanged(int arg1);

private:
    void emitPointCoordsEdited();

    Ui::ControlsDialog *ui;

    bool settingPointCoords_{false};
};

#endif // CONTROLSDIALOG_H
