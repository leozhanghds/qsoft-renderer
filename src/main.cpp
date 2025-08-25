#include <iostream>

#include <QApplication>
#include "RenderWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    RenderWidget renderWidget;
    renderWidget.show();

    renderWidget.render();

    return app.exec();
}