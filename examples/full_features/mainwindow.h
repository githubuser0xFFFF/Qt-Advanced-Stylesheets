#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct MainWindowPrivate;

class CMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    CMainWindow(QWidget *parent = nullptr);
    virtual ~CMainWindow();

private:
    MainWindowPrivate* d;
    friend struct MainWindowPrivate;// pimpl

private slots:
	void onThemeActionTriggered();
	void onStyleManagerStylesheetChanged();
	void onThemeColorButtonClicked();
};
#endif // CMAINWINDOW_H
