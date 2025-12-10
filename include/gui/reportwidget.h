#ifndef REPORTWIDGET_H
#define REPORTWIDGET_H
#include <QWidget>
#include <QDate>
#include <QTableView>

class QPushButton;
class QDateEdit;
class QStandardItemModel;

class ReportWidget : public QWidget
{
    Q_OBJECT  // Важно для работы сигналов/слотов!

public:
    explicit ReportWidget(QWidget *parent = nullptr);
    void updateReport();
    
private slots:
    void exportToCsv();   // Экспорт (опционально)
    void exportToPdf();
    
private:
    void setupUI();       // Настройка интерфейса
    void setupTable();    // Настройка таблицы
    
    // Элементы интерфейса
    QTableView *tableView;
    QDateEdit *dateStartEdit;
    QDateEdit *dateEndEdit;
    QPushButton *updateButton;
    QPushButton *exportButton;
    
    // Модель данных для таблицы
    QStandardItemModel *model;
};

#endif // REPORTWIDGET_H