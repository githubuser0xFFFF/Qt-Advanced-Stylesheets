#include "mainwindow.h"

#include <StyleManager.h>

#include "ui_mainwindow.h"
#include <QDir>
#include <QApplication>
#include <QAction>
#include <QListWidgetItem>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QColorDialog>

#include <iostream>


/**
 * Private data class - pimpl
 */
struct MainWindowPrivate
{
	CMainWindow* _this;
	Ui::MainWindow ui;
	acss::CStyleManager* StyleManager;
	QVector<QPushButton*> ThemeColorButtons;

	/**
	 * Private data constructor
	 */
	MainWindowPrivate(CMainWindow* _public) : _this(_public) {}

	void createThemeColorDockWidget();
	void fillThemeMenu();
	void setSomeIcons();
	void updateThemeColorButtons();
};


void MainWindowPrivate::createThemeColorDockWidget()
{
	QDockWidget* dock = new QDockWidget(("Change Theme"), _this);
	QWidget* w = new QWidget(dock);
	auto Layout = new QVBoxLayout(w);
	Layout->setContentsMargins(12, 12, 12, 12);
	Layout->setSpacing(12);
	w->setLayout(Layout);
	dock->setWidget(w);
	_this->addDockWidget(Qt::LeftDockWidgetArea, dock);
	dock->setFloating(true);

	const auto& ThemeColors = StyleManager->themeColorVariables();
	for (auto itc = ThemeColors.constBegin(); itc != ThemeColors.constEnd(); ++itc)
	{
		auto Button = new QPushButton(itc.key());
		QObject::connect(Button, &QPushButton::clicked, _this, &CMainWindow::onThemeColorButtonClicked);
		Layout->addWidget(Button);
		ThemeColorButtons.append(Button);
	}

	updateThemeColorButtons();
}


void MainWindowPrivate::updateThemeColorButtons()
{
	for (auto Button : ThemeColorButtons)
	{
		auto Color = StyleManager->themeColor(Button->text());
		QString TextColor = (Color.value() < 128) ? "#ffffff" : "#000000";
		QString ButtonStylesheet = QString("background-color: %1; color: %2;"
			"border: none;").arg(Color.name()).arg(TextColor);
		Button->setStyleSheet(ButtonStylesheet);
	}
}



void MainWindowPrivate::fillThemeMenu()
{
    // Add actions for theme selection
    auto m = ui.menuThemes;
    for (const auto& Theme : StyleManager->themes())
    {
    	QAction* a = new QAction(Theme);
    	m->addAction(a);
    	QObject::connect(a, &QAction::triggered, _this, &CMainWindow::onThemeActionTriggered);
    }

}


void MainWindowPrivate::setSomeIcons()
{
    ui.actionToolbar->setIcon(StyleManager->styleIcon());
    QIcon Icon(":/full_features/images/logo_frame.svg");
    for (int i = 0; i < ui.listWidget_2->count(); ++i)
    {
    	ui.listWidget_2->item(i)->setIcon(Icon);
    }
}


CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow(parent),
      d(new MainWindowPrivate(this))
{
    d->ui.setupUi(this);

    QString AppDir = qApp->applicationDirPath();
    d->StyleManager = new acss::CStyleManager(this);
    d->StyleManager->setStylesDirPath(AppDir + "/../../styles");
    d->StyleManager->setOutputDirPath(AppDir + "/output");
    d->StyleManager->setCurrentStyle("qt_material");
    d->StyleManager->setCurrentTheme("dark_teal");
    d->StyleManager->updateStylesheet();
    setWindowIcon(d->StyleManager->styleIcon());
    qApp->setStyleSheet(d->StyleManager->styleSheet());
    connect(d->StyleManager, SIGNAL(stylesheetChanged()), this,
    	SLOT(onStyleManagerStylesheetChanged()));

    d->createThemeColorDockWidget();
    d->fillThemeMenu();
    d->setSomeIcons();
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
	d->updateThemeColorButtons();
}


void CMainWindow::onThemeColorButtonClicked()
{
	auto Button = qobject_cast<QPushButton*>(sender());
	QColorDialog ColorDialog;
	auto Color = d->StyleManager->themeColor(Button->text());
	ColorDialog.setCurrentColor(Color);
	if (ColorDialog.exec() != QDialog::Accepted)
	{
		return;
	}
	Color = ColorDialog.currentColor();
	d->StyleManager->setThemeVariableValue(Button->text(), Color.name());
	d->StyleManager->updateStylesheet();
}

