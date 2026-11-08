#include <QApplication>
#include <QFont>
#include "mainwindow.h"
#include "theme.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("BarcodeHub");
    app.setStyle("Fusion");                              // consistent base on every OS

    QFont f;
    f.setFamilies({"Inter", "Segoe UI", "Noto Sans", "Helvetica Neue", "Arial"});
    f.setPixelSize(13);
    app.setFont(f);
    app.setStyleSheet(Theme::styleSheet());

    MainWindow w;
    w.show();
    return app.exec();
}
