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
    void generationsEdited(size_t generations);
    void antialiasingChanged(bool enabled);
    void fancyPenChanged(bool enabled);
    void allGenerationsChanged(bool enabled);
    void approxAlgorithmChanged(bool enabled);
    void approxBboxGenEdited(size_t bboxGen);
    void approxMaxGenEdited(size_t maxGen);
    void approxMaxVerticesEdited(size_t maxVertices);

public slots:
    void setPointCoords(double x, double y);
    void disablePoint();
    void setGenerations(size_t generations);
    void setAntialiasing(bool enabled);
    void setFancyPen(bool enabled);
    void setAllGenerations(bool enabled);
    void setApproxAlgorithm(bool enabled);
    void setApproxBboxGen(size_t bboxGen);
    void setApproxMaxGen(size_t maxGen);
    void setApproxMaxVertices(size_t maxVertices);

private slots:
    void on_generations_valueChanged(int arg1);
    void on_edit_x_valueChanged(double arg1);
    void on_edit_y_valueChanged(double arg1);

    void on_checkAntialiasing_stateChanged(int arg1);
    void on_checkPen_stateChanged(int arg1);
    void on_checkAllGen_stateChanged(int arg1);
    void on_checkApprox_stateChanged(int arg1);

    void on_spinApproxBboxGen_valueChanged(int arg1);

    void on_spinApproxMaxGen_valueChanged(int arg1);

    void on_spinApproxMaxVertices_valueChanged(int arg1);

private:
    void emitPointCoordsEdited();

    Ui::ControlsDialog *ui;

    bool settingPointCoords_{false};
};

#endif // CONTROLSDIALOG_H
