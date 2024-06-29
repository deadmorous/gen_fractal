#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class Document;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

public slots:
    auto open(const QString& fileName)
        -> void;

private:
    Document* doc_;
};
#endif // MAINWINDOW_H
