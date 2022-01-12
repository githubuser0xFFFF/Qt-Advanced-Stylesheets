#include "MainWindow.h"

#include <QtAdvancedStylesheet.h>
#include <QmlStyleUrlInterceptor.h>

#include "ui_MainWindow.h"
#include <QDir>
#include <QApplication>
#include <QAction>
#include <QListWidgetItem>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QColorDialog>
#include <QDebug>
#include <QQmlEngine>

#include <iostream>


#define _STR(x) #x
#define STRINGIFY(x)  _STR(x)

/**
 * Private data class - pimpl
 */
struct MainWindowPrivate
{
	CMainWindow* _this;
	Ui::MainWindow ui;
	acss::QtAdvancedStylesheet* AdvancedStyleSheet;
	QVector<QPushButton*> ThemeColorButtons;

	/**
	 * Private data constructor
	 */
	MainWindowPrivate(CMainWindow* _public) : _this(_public) {}

	void createThemeColorDockWidget();
	void fillThemeMenu();
	void setSomeIcons();
	void setupQuickWidget();
	void updateThemeColorButtons();
	void updateQuickWidget();
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

	const auto& ThemeColors = AdvancedStyleSheet->themeColorVariables();
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
		auto Color = AdvancedStyleSheet->themeColor(Button->text());
		QString TextColor = (Color.value() < 128) ? "#ffffff" : "#000000";
		QString ButtonStylesheet = QString("background-color: %1; color: %2;"
			"border: none;").arg(Color.name()).arg(TextColor);
		Button->setStyleSheet(ButtonStylesheet);
	}
}


void MainWindowPrivate::updateQuickWidget()
{
	const auto Source = ui.quickWidget->source();
	ui.quickWidget->setSource({});
	ui.quickWidget->engine()->clearComponentCache();
	ui.quickWidget->setSource(Source);
	ui.quickWidget->setStyleSheet(AdvancedStyleSheet->styleSheet());
}



void MainWindowPrivate::fillThemeMenu()
{
    // Add actions for theme selection
    auto m = ui.menuThemes;
    for (const auto& Theme : AdvancedStyleSheet->themes())
    {
    	QAction* a = new QAction(Theme);
    	m->addAction(a);
    	QObject::connect(a, &QAction::triggered, _this, &CMainWindow::onThemeActionTriggered);
    }

}


void MainWindowPrivate::setSomeIcons()
{
    ui.actionToolbar->setIcon(AdvancedStyleSheet->styleIcon());
    QIcon Icon(":/full_features/images/logo_frame.svg");
    for (int i = 0; i < ui.listWidget_2->count(); ++i)
    {
    	ui.listWidget_2->item(i)->setIcon(Icon);
    }
}

void MainWindowPrivate::setupQuickWidget()
{
    ui.quickWidget->engine()->setUrlInterceptor(
        new acss::CQmlStyleUrlInterceptor(AdvancedStyleSheet));
    ui.quickWidget->setStyleSheet(AdvancedStyleSheet->styleSheet());
    ui.quickWidget->setSource(QUrl("qrc:/full_features/qml/simple_demo.qml"));
}


CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow(parent),
      d(new MainWindowPrivate(this))
{
    d->ui.setupUi(this);

    QString AppDir = qApp->applicationDirPath();
    QString StylesDir = STRINGIFY(STYLES_DIR);
    d->AdvancedStyleSheet = new acss::QtAdvancedStylesheet(this);
    d->AdvancedStyleSheet->setStylesDirPath(StylesDir);
    d->AdvancedStyleSheet->setOutputDirPath(AppDir + "/output");
    d->AdvancedStyleSheet->setCurrentStyle("qt_material");
    d->AdvancedStyleSheet->setCurrentTheme("dark_teal");
    d->AdvancedStyleSheet->updateStylesheet();
    setWindowIcon(d->AdvancedStyleSheet->styleIcon());
    qApp->setStyleSheet(d->AdvancedStyleSheet->styleSheet());
    connect(d->AdvancedStyleSheet, SIGNAL(stylesheetChanged()), this,
    	SLOT(onStyleManagerStylesheetChanged()));

    d->createThemeColorDockWidget();
    d->fillThemeMenu();
    d->setSomeIcons();
    d->setupQuickWidget();
}

CMainWindow::~CMainWindow()
{
	delete d;
}


void CMainWindow::onThemeActionTriggered()
{
	auto Action = qobject_cast<QAction*>(sender());
	d->AdvancedStyleSheet->setCurrentTheme(Action->text());
	d->AdvancedStyleSheet->updateStylesheet();
}


void CMainWindow::onStyleManagerStylesheetChanged()
{
	qApp->setStyleSheet(d->AdvancedStyleSheet->styleSheet());
	d->updateThemeColorButtons();
	d->updateQuickWidget();
}


void CMainWindow::onThemeColorButtonClicked()
{
	auto Button = qobject_cast<QPushButton*>(sender());
	QColorDialog ColorDialog;
	auto Color = d->AdvancedStyleSheet->themeColor(Button->text());
	ColorDialog.setCurrentColor(Color);
	if (ColorDialog.exec() != QDialog::Accepted)
	{
		return;
	}
	Color = ColorDialog.currentColor();
	d->AdvancedStyleSheet->setThemeVariableValue(Button->text(), Color.name());
	d->AdvancedStyleSheet->updateStylesheet();
}

