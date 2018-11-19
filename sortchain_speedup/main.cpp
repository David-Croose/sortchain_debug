#include "mainwindow.h"
#include <QApplication>

extern "C" {extern int sortchain_demo_main(void);}

int main(int argc, char *argv[])
{
#if 0
    sortchain_demo_main();
    return 0;
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
