#include <QApplication>
#include <QDir>

#include "mainwindow.h"
#include "AdvancedStylesheet.h"

#include <iostream>

using namespace acss;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDir StylesheetDir(a.applicationDirPath() + "/../../stylesheets/qt_material");
    std::cout << StylesheetDir.absolutePath().toStdString() << std::endl;
    CAdvancedStylesheet Stylesheet(StylesheetDir.absolutePath());
    Stylesheet.setOutputDirPath(a.applicationDirPath() + "/output");
    Stylesheet.setTheme("dark_cyan.xml");
    a.setStyleSheet(Stylesheet.styleSheet());

    CMainWindow w;
    w.show();
    return a.exec();
}
