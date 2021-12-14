#include <QCoreApplication>
#include <QDir>

#include <iostream>
#include "AdvancedStylesheet.h"

using namespace acss;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    std::cout << "Hello World!" << std::endl;
    std::cout << a.applicationDirPath().toStdString() << std::endl;

    QDir StylesheetDir(a.applicationDirPath() + "/../../stylesheets/qt_material");
    std::cout << StylesheetDir.absolutePath().toStdString() << std::endl;
    CAdvancedStylesheet Stylesheet(StylesheetDir.absolutePath());
    Stylesheet.setOutputDirPath(a.applicationDirPath() + "/output");

    Stylesheet.setTheme("dark_cyan.xml");
}
