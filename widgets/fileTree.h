#ifndef FILETREE_H
#define FILETREE_H

#include <QFileSystemModel>
#include <QLabel>
#include <QTreeView>

enum FileOperation { OPEN, OPEN_LOCALLY, RENAME, DELETE };

class FileTreeWidget : public QWidget {
    Q_OBJECT

    QTreeView *treeView;
    QFileSystemModel *model;
    QLabel *headerLabel;

    void setup();
    /* Add file operation to the context menu */
    void addFileOperationToMenu(QMenu &menu, const QString &file, const QString &label, FileOperation operation);

signals:
    /* The user wants to operate the file */
    void rawOperateFile(const QString &filename, FileOperation operation);
    /* Only when the operation is successful, emit this signal (public to other widgets) */
    void operateFile(const QString &filename, FileOperation operation);

private slots:
    /* Click on a file to open it */
    void clickFile(const QModelIndex &index);
    /* Handle the file operation */
    void handleRawFileOperation(const QString &filename, FileOperation operation);

public slots:
    /* Create a new file in the directory */
    void createNewFile(const QString &dir);
    /* Create a new folder in the directory */
    void createNewFolder(const QString &dir);

protected:
    // override methods for personalized behavior
    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

public:
    static QMap<Qt::Key, FileOperation> opShortcuts;

    explicit FileTreeWidget(QWidget *parent = nullptr);
    void setRoot(const QString &root);
};


#endif // FILETREE_H
