#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MdiChild;
class QMdiSubWindow;
class QSignalMapper;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void updateMenus();

    MdiChild *createMdiChild();
    void setActiveSubWindow(QWidget *window);
    void updateWindowMenu();
    void showTextRowAanCol();

private:
    Ui::MainWindow *ui;

    QAction *actionSeparator;
    QSignalMapper *windowMapper;
    MdiChild *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);
    void readSettings();
    void writeSettings();
    void initWindow();

private slots:

    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExit_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionClose_triggered();
    void on_actionCloseAll_triggered();
    void on_actionTile_triggered();
    void on_actionCasecade_triggered();
    void on_actionNext_triggered();
    void on_actionPrevious_triggered();
    void on_actionAboutQt_triggered();
    void on_actionAbout_triggered();
};
#endif // MAINWINDOW_H
