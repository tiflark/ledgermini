#include "gui/tableactions.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QKeySequence>

TableActions::TableActions(QTableView *tableView, QObject *parent)
    : QObject(parent), tableView_(tableView)
{
    setupContextMenu();
}

void TableActions::setupContextMenu()
{
    tableView_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableView_, &QTableView::customContextMenuRequested,
            this, &TableActions::onContextMenu);
    
    contextMenu_ = new QMenu(tableView_);
    
    addAction_ = contextMenu_->addAction("Добавить");
    addAction_->setShortcut(QKeySequence("Ctrl+N"));
    connect(addAction_, &QAction::triggered, this, &TableActions::onAddAction);
    
    editAction_ = contextMenu_->addAction("Редактировать");
    editAction_->setShortcut(QKeySequence("F2"));
    connect(editAction_, &QAction::triggered, this, &TableActions::onEditAction);
    
    deleteAction_ = contextMenu_->addAction("Удалить");
    deleteAction_->setShortcut(QKeySequence::Delete);
    connect(deleteAction_, &QAction::triggered, this, &TableActions::onDeleteAction);
    
    contextMenu_->addSeparator();
    
    refreshAction_ = contextMenu_->addAction("Обновить");
    refreshAction_->setShortcut(QKeySequence::Refresh);
    connect(refreshAction_, &QAction::triggered, this, &TableActions::onRefreshAction);

    contextMenu_->addSeparator();
    exportPdfAction_ = contextMenu_->addAction("Экспорт в PDF");
    connect(exportPdfAction_, &QAction::triggered, this, &TableActions::onExportToPdf);
}

void TableActions::onContextMenu(const QPoint &pos)
{
    // Всегда показываем контекстное меню, даже если нет выбранной строки
    contextMenu_->exec(tableView_->viewport()->mapToGlobal(pos));
}

int TableActions::getSelectedId() const
{
    QModelIndex index = tableView_->currentIndex();
    if (!index.isValid()) return -1;
    
    // ID обычно в скрытой первой колонке
    return tableView_->model()->index(index.row(), 0).data().toInt();
}

void TableActions::onAddAction()
{
    emit addRequested();
}

void TableActions::onEditAction()
{
    int id = getSelectedId();
    if (id > 0) {
        emit editRequested(id);
    } else {
        QMessageBox::information(tableView_, "Информация", 
            "Выберите запись для редактирования.");
    }
}

void TableActions::onDeleteAction()
{
    int id = getSelectedId();
    if (id <= 0) {
        QMessageBox::information(tableView_, "Информация", 
            "Выберите запись для удаления.");
        return;
    }
    
    emit deleteRequested(id);
}

void TableActions::onRefreshAction()
{
    emit refreshRequested();
}

void TableActions::setAddEnabled(bool enabled) {
    addAction_->setEnabled(enabled);
}

void TableActions::setEditEnabled(bool enabled) {
    editAction_->setEnabled(enabled);
}

void TableActions::setDeleteEnabled(bool enabled) {
    deleteAction_->setEnabled(enabled);
}

void TableActions::onExportToPdf()
{
    emit exportToPdfRequested();
}