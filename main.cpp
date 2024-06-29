#include "mainwindow.h"

#include "batch.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto args = a.arguments();

    if (args.size() == 3 && args[1] == "--batch")
        return batch(args[2]);;

    MainWindow w;
    w.show();

    if (args.size() == 2)
    { w.open(args[1]); }

    return a.exec();
}
