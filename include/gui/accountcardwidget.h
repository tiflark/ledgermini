#ifndef ACCOUNTCARDWIDGET_H
#define ACCOUNTCARDWIDGET_H

#include <QWidget>

class QComboBox;
class QDateEdit;
class QTableView;
class QPushButton;
class QStandardItemModel;

class AccountCardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AccountCardWidget(QWidget *parent = nullptr);
    
private slots:
    void updateReport();
    void exportToCsv();
    void loadAccounts();

private:
    void setupUI();
    void setupTable();
    
    // Элементы интерфейса
    QComboBox *accountCombo;
    QDateEdit *dateStartEdit;
    QDateEdit *dateEndEdit;
    QTableView *tableView;
    QPushButton *updateButton;
    QPushButton *exportButton;
    
    // Модель данных
    QStandardItemModel *model;
};

#endif // ACCOUNTCARDWIDGET_H