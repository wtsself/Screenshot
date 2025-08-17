#include "ScreenshotWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ScreenshotWidget w;
    w.show();
    return a.exec();
}
