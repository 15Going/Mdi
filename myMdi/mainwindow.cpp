#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mdichild.h"

#include <QMdiSubWindow>
#include <QFileDialog>
#include <QSignalMapper>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    actionSeparator = new QAction(this);
    actionSeparator->setSeparator(true);

    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mappedObject(QWidget*)), // 映射器重新发射信号
                this, SLOT(setActiveSubWindow(QWidget*))); // 设置活动窗口

    updateWindowMenu();

    updateMenus();

    readSettings();

    connect(ui->mdiArea,&QMdiArea::subWindowActivated,
            this,&MainWindow::updateMenus);

    connect(ui->menuW,SIGNAL(aboutToShow()),this,SLOT(updateWindowMenu()));

    initWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    ui->mdiArea->closeActiveSubWindow();
    if(ui->mdiArea->currentSubWindow())
        event->ignore();
    else{
        writeSettings();
        event->accept();
    }
}


void MainWindow::on_actionNew_triggered()
{
    MdiChild *child = createMdiChild();
    //ui->mdiArea->addSubWindow(child);
    child->newFile();
    child->show();
}

MdiChild *MainWindow::activeMdiChild()
{
    if(QMdiSubWindow *activeSubWindow = ui->mdiArea->activeSubWindow())
        return qobject_cast<MdiChild*>(activeSubWindow->widget());
    return 0;
}

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    foreach (QMdiSubWindow *window, ui->mdiArea->subWindowList()) {
        MdiChild *child = qobject_cast<MdiChild*>(window->widget());
        if(child->currentFile() == canonicalFilePath)
            return window;
    }
    return 0;
}

void MainWindow::readSettings()
{
    QSettings settings("yafeilinux","myMdi");
    QPoint pos = settings.value("pos",QPoint(200,200)).toPoint();
    QSize size = settings.value("size",QSize(400,400)).toSize();
    move(pos);
    resize(size);
}

void MainWindow::writeSettings()
{
    QSettings settings("yafeilinux","myMdi");
    settings.setValue("pos",pos());
    settings.setValue("size",size());
}

void MainWindow::initWindow()
{
    setWindowTitle(tr("多文档编辑器"));
    ui->toolBar->setWindowTitle(tr("工具栏"));
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->statusbar->showMessage(tr("欢迎使用多文档编辑器"));

    QLabel *label = new QLabel(this);
    label->setFrameStyle(QFrame::Box | QFrame::Sunken);
    label->setText(tr("<a href = \"http://www.yafeilinux.com\">yafeilinux.com</a>"));
    label->setTextFormat(Qt::RichText);
    label->setOpenExternalLinks(true);
    ui->statusbar->addPermanentWidget(label);
    ui->actionNew->setStatusTip(tr("创建一个文件"));
}

void MainWindow::updateMenus()
{
    bool hasMdiChild = (activeMdiChild() != 0);
    ui->actionSave->setEnabled(hasMdiChild);
    ui->actionSaveAs->setEnabled(hasMdiChild);
    ui->actionPaste->setEnabled(hasMdiChild);
    ui->actionClose->setEnabled(hasMdiChild);
    ui->actionCloseAll->setEnabled(hasMdiChild);
    ui->actionTile->setEnabled(hasMdiChild);
    ui->actionCasecade->setEnabled(hasMdiChild);
    ui->actionNext->setEnabled(hasMdiChild);
    ui->actionPrevious->setEnabled(hasMdiChild);

    actionSeparator->setVisible(hasMdiChild);

    bool hasSelection = (activeMdiChild() && activeMdiChild()->textCursor()
                         .hasSelection());
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);

    ui->actionUndo->setEnabled(activeMdiChild() && activeMdiChild()->document()
                               ->isUndoAvailable());
    ui->actionRedo->setEnabled(activeMdiChild() && activeMdiChild()->document()
                               ->isRedoAvailable());
}

MdiChild *MainWindow::createMdiChild()
{
    MdiChild *child = new MdiChild;
    ui->mdiArea->addSubWindow(child);

    connect(child,&MdiChild::copyAvailable,ui->actionCut,&QAction::setEnabled);
    connect(child,&MdiChild::copyAvailable,ui->actionCopy,&QAction::setEnabled);
    connect(child->document(),&QTextDocument::undoAvailable,
            ui->actionUndo,&QAction::setEnabled);
    connect(child->document(),&QTextDocument::redoAvailable,
            ui->actionRedo,&QAction::setEnabled);
    connect(child,&MdiChild::cursorPositionChanged,
            this,&MainWindow::showTextRowAanCol);
    return child;
}

void MainWindow::setActiveSubWindow(QWidget *window)
{
    if(!window)
        return;
    ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(window));
}

void MainWindow::updateWindowMenu()
{
    ui->menuW->clear();
    ui->menuW->addAction(ui->actionClose);
    ui->menuW->addAction(ui->actionCloseAll);
    ui->menuW->addSeparator();
    ui->menuW->addAction(ui->actionTile);
    ui->menuW->addAction(ui->actionCasecade);
    ui->menuW->addSeparator();
    ui->menuW->addAction(ui->actionNext);
    ui->menuW->addAction(ui->actionPrevious);
    ui->menuW->addAction(actionSeparator);

    QList<QMdiSubWindow*>windows = ui->mdiArea->subWindowList();
    actionSeparator->setVisible(!windows.isEmpty());

    for(int i = 0;i < windows.size(); ++i){
        MdiChild *child = qobject_cast<MdiChild*>(windows.at(i)->widget());
        QString text;
        if(i < 9){
            text = tr("&%1 %2").arg(i + 1)
                    .arg(child->userFriendlyCurrentFile());
        }else{
            text = tr("%1 %2").arg(i + 1)
                    .arg(child->userFriendlyCurrentFile());
        }

        QAction *action = ui->menuW->addAction(text);
        action->setCheckable(true);
        action->setChecked(child == activeMdiChild());
        connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
        windowMapper->setMapping(action,windows.at(i));
    }
}

void MainWindow::showTextRowAanCol()
{
    if(activeMdiChild()){
        int rowNum = activeMdiChild()->textCursor().blockNumber() + 1;
        int colNum = activeMdiChild()->textCursor().columnNumber() + 1;
        ui->statusbar->showMessage(tr("%1行%2列")
                                   .arg(rowNum).arg(colNum),2000);
    }
}


void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if(!fileName.isEmpty()){
        QMdiSubWindow *existing = findMdiChild(fileName);
        if(existing){
            ui->mdiArea->setActiveSubWindow(existing);
            return;
        }
        MdiChild *child = createMdiChild();
        if(child->loadFile(fileName)){
            ui->statusbar->showMessage(tr("打开文件成功"),2000);
            child->show();
        }else{
            child->close();
        }
    }
}


void MainWindow::on_actionSave_triggered()
{
    if(activeMdiChild() && activeMdiChild()->save())
        ui->statusbar->showMessage(tr("文件保存成功"),2000);
}


void MainWindow::on_actionSaveAs_triggered()
{
    if(activeMdiChild() && activeMdiChild()->saveAs())
        ui->statusbar->showMessage(tr("文件保存成功"),2000);
}


void MainWindow::on_actionExit_triggered()
{
    //close();
    qApp->closeAllWindows();
}


void MainWindow::on_actionUndo_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->undo();
}


void MainWindow::on_actionRedo_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->redo();
}


void MainWindow::on_actionCut_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->cut();
}


void MainWindow::on_actionCopy_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->copy();
}


void MainWindow::on_actionPaste_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->paste();
}


void MainWindow::on_actionClose_triggered()
{
    ui->mdiArea->closeActiveSubWindow();
}


void MainWindow::on_actionCloseAll_triggered()
{
    ui->mdiArea->closeAllSubWindows();
}


void MainWindow::on_actionTile_triggered()
{
    ui->mdiArea->tileSubWindows();
}


void MainWindow::on_actionCasecade_triggered()
{
    ui->mdiArea->cascadeSubWindows();
}


void MainWindow::on_actionNext_triggered()
{
    ui->mdiArea->activateNextSubWindow();
}


void MainWindow::on_actionPrevious_triggered()
{
    ui->mdiArea->activatePreviousSubWindow();
}


void MainWindow::on_actionAboutQt_triggered()
{
    qApp->aboutQt();
}


void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,tr("关于本软件"),tr("欢迎访问我们的网站：www.yafeilinux.com"));
}

