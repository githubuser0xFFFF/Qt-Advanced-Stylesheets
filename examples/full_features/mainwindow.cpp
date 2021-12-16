#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QApplication>
#include <QAction>
#include <QListWidgetItem>

#include "AdvancedStylesheet.h"
#include <iostream>


/**
 * Private data class - pimpl
 */
struct MainWindowPrivate
{
	CMainWindow* _this;
	Ui::MainWindow ui;
	acss::CStyleManager* StyleManager;
};


CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow(parent),
      d(new MainWindowPrivate())
{
	std::cout << "Test: " << std::endl;
    d->ui.setupUi(this);

    QString AppDir = qApp->applicationDirPath();
    d->StyleManager = new acss::CStyleManager(this);
    d->StyleManager->setStylesDirPath(AppDir + "/../../stylesheets");
    d->StyleManager->setOutputDirPath(AppDir + "/output");
    d->StyleManager->setCurrentStyle("qt_material");
    d->StyleManager->setCurrentTheme("dark_teal");
    setWindowIcon(d->StyleManager->styleIcon());
    qApp->setStyleSheet(d->StyleManager->styleSheet());
    connect(d->StyleManager, SIGNAL(stylesheetChanged()), this,
    	SLOT(onStyleManagerStylesheetChanged()));

    // Add actions for theme selection
    auto m = d->ui.menuThemes;
    for (const auto& Theme : d->StyleManager->themes())
    {
    	QAction* a = new QAction(Theme);
    	m->addAction(a);
    	connect(a, &QAction::triggered, this, &CMainWindow::onThemeActionTriggered);
    }

    d->ui.actionToolbar->setIcon(d->StyleManager->styleIcon());
    QIcon Icon(":/full_features/images/logo_frame.svg");
    for (int i = 0; i < d->ui.listWidget_2->count(); ++i)
    {
    	d->ui.listWidget_2->item(i)->setIcon(Icon);
    }
}

CMainWindow::~CMainWindow()
{
	delete d;
}


void CMainWindow::onThemeActionTriggered()
{
	auto Action = qobject_cast<QAction*>(sender());
	d->StyleManager->setCurrentTheme(Action->text());
}


void CMainWindow::onStyleManagerStylesheetChanged()
{
	qApp->setStyleSheet(d->StyleManager->styleSheet());
}

