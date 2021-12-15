#include <QApplication>
#include <QDir>

#include "mainwindow.h"
#include "AdvancedStylesheet.h"

#include <iostream>

using namespace acss;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDir StylesDir(a.applicationDirPath() + "/../../stylesheets");
    std::cout << StylesDir.absolutePath().toStdString() << std::endl;

    CAdvancedStylesheet Stylesheet;
    Stylesheet.setStylesDir(StylesDir.absolutePath());
    Stylesheet.setOutputDirPath(a.applicationDirPath() + "/output");
    Stylesheet.setCurrentStyle("qt_material");
    Stylesheet.setTheme("dark_cyan.xml");

    CMainWindow w;
    w.setWindowIcon(Stylesheet.styleIcon());
    w.show();

    a.setStyleSheet(Stylesheet.styleSheet());
    return a.exec();
}
