#ifndef TABLEACTIONS_H
#define TABLEACTIONS_H

#include <QObject>
#include <QTableView>
#include <QMenu>

class TableActions : public QObject
{
    Q_OBJECT

public:
    explicit TableActions(QTableView *tableView, QObject *parent = nullptr);
    
    void setupContextMenu();
    void setAddEnabled(bool enabled);
    void setEditEnabled(bool enabled);
    void setDeleteEnabled(bool enabled);
    
signals:
    void addRequested();
    void editRequested(int id);
    void deleteRequested(int id);
    void refreshRequested();
    void exportToPdfRequested();

private slots:
    void onContextMenu(const QPoint &pos);
    void onAddAction();
    void onEditAction();
    void onDeleteAction();
    void onRefreshAction();
    void onExportToPdf();


private:
    QTableView *tableView_;
    QMenu *contextMenu_;
    QAction *addAction_;
    QAction *editAction_;
    QAction *deleteAction_;
    QAction *refreshAction_;
    QAction *exportPdfAction_;

    
    int getSelectedId() const;
};

#endif // TABLEACTIONS_H