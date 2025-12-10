#ifndef OPERATIONSJOURNALWIDGET_H
#define OPERATIONSJOURNALWIDGET_H

#include <QWidget>
#include <QLabel>
#include "gui/advancedfilterwidget.h"

class QTableView;
class QStandardItemModel;
class QPushButton;

class OperationsJournalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OperationsJournalWidget(QWidget *parent = nullptr);
    
    void refreshJournal();
    void applyFilter(const AdvancedFilterWidget::FilterOptions &options);

private slots:
    void onFilterApplied();
    void exportToPdf();
    void exportToExcel();

private:
    void setupUI();
    void loadData();
    void initConnections();
    QString buildQuery(const AdvancedFilterWidget::FilterOptions &options);
    
    AdvancedFilterWidget *filterWidget;
    QTableView *tableView;
    QStandardItemModel *model;
    QPushButton *refreshButton;
    QPushButton *pdfButton;
    QPushButton *excelButton;
    QLabel *statusLabel;
};

#endif // OPERATIONSJOURNALWIDGET_H