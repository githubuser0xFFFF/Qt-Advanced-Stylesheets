#include <QtAdvancedStylesheet.h>
#include <QCoreApplication>
#include <QDir>

#include <iostream>

using namespace acss;

#define _STR(x) #x
#define STRINGIFY(x)  _STR(x)

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QtAdvancedStylesheet AdvancedStylesheet;
    QString AppDir = qApp->applicationDirPath();
    QString StylesDir = STRINGIFY(STYLES_DIR);
    AdvancedStylesheet.setStylesDirPath(StylesDir);
    AdvancedStylesheet.setOutputDirPath(AppDir + "/output");
    AdvancedStylesheet.setCurrentStyle("qt_material");
    AdvancedStylesheet.setCurrentTheme("dark_teal");
}
