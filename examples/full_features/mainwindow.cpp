#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QApplication>

#include "AdvancedStylesheet.h"
#include <iostream>

CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString AppDir = qApp->applicationDirPath();
    auto StyleManager = new acss::CStyleManager(this);
    StyleManager->setStylesDir(AppDir + "/../../stylesheets");
    StyleManager->setOutputDirPath(AppDir + "/output");
    StyleManager->setCurrentStyle("qt_material");
    StyleManager->setTheme("dark_cyan.xml");
    setWindowIcon(StyleManager->styleIcon());
    qApp->setStyleSheet(StyleManager->styleSheet());
}

CMainWindow::~CMainWindow()
{
    delete ui;
}

