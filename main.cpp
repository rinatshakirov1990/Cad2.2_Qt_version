#include <QApplication>
#include "paintdemo.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    PaintDemo demo;
    demo.show();

    return app.exec();
}
