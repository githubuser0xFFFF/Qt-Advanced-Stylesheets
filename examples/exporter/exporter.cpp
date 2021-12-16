#include <StyleManager.h>
#include <QCoreApplication>
#include <QDir>

#include <iostream>

using namespace acss;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    std::cout << "Hello World!" << std::endl;
    std::cout << a.applicationDirPath().toStdString() << std::endl;

    QDir StylesDir(a.applicationDirPath() + "/../../stylesheets");
    std::cout << StylesDir.absolutePath().toStdString() << std::endl;
    CStyleManager Stylesheet;
    Stylesheet.setStylesDirPath(StylesDir.absolutePath());
    Stylesheet.setOutputDirPath(a.applicationDirPath() + "/output");

    Stylesheet.setCurrentTheme("dark_cyan.xml");
}
