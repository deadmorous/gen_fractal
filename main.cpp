#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    auto args = a.arguments();
    if (args.size() == 2)
    {
        w.open(args[1]);
    }

    return a.exec();
}
