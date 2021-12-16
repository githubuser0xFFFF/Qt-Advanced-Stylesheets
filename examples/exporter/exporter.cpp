#include <StyleManager.h>
#include <QCoreApplication>
#include <QDir>

#include <iostream>

using namespace acss;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    CStyleManager StyleManager;
    QString AppDir = qApp->applicationDirPath();
    StyleManager.setStylesDirPath(AppDir + "/../../styles");
    StyleManager.setOutputDirPath(AppDir + "/output");
    StyleManager.setCurrentStyle("qt_material");
    StyleManager.setCurrentTheme("dark_teal");
}
