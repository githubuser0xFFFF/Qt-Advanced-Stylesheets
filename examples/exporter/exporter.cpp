#include <StyleManager.h>
#include <QCoreApplication>
#include <QDir>

#include <iostream>

using namespace acss;

#define _STR(x) #x
#define STRINGIFY(x)  _STR(x)

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    CStyleManager StyleManager;
    QString AppDir = qApp->applicationDirPath();
    QString StylesDir = STRINGIFY(STYLES_DIR);
    StyleManager.setStylesDirPath(StylesDir);
    StyleManager.setOutputDirPath(AppDir + "/output");
    StyleManager.setCurrentStyle("qt_material");
    StyleManager.setCurrentTheme("dark_teal");
}
