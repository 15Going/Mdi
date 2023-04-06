#ifndef MDICHILD_H
#define MDICHILD_H

#include <QTextEdit>

class MdiChild : public QTextEdit
{
    Q_OBJECT
public:
    explicit MdiChild(QWidget *parent = 0);

    void newFile();
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    QString userFriendlyCurrentFile();
    QString currentFile()
    {
        return curFile;
    }

protected:
    void closeEvent(QCloseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void documemtWasModified();

private:
    bool isUntitled;
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    QString curFile;
};

#endif // MDICHILD_H
