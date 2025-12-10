#include <QStringConverter>
#include "gui/reportwidget.h"
#include "core/report_generator.h"
#include "core/exportmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateEdit>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>

ReportWidget::ReportWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupTable();
    
    // Устанавливаем период по умолчанию (текущий месяц)
    QDate today = QDate::currentDate();
    dateStartEdit->setDate(QDate(today.year(), today.month(), 1));
    dateEndEdit->setDate(today);
    
    // Обновляем отчет при запуске
    updateReport();
}

void ReportWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 1. Панель управления (фильтры и кнопки)
    QHBoxLayout *controlLayout = new QHBoxLayout();
    
    controlLayout->addWidget(new QLabel("Период с:"));
    dateStartEdit = new QDateEdit;
    dateStartEdit->setCalendarPopup(true);
    controlLayout->addWidget(dateStartEdit);
    
    controlLayout->addWidget(new QLabel("по:"));
    dateEndEdit = new QDateEdit;
    dateEndEdit->setCalendarPopup(true);
    controlLayout->addWidget(dateEndEdit);
    
    updateButton = new QPushButton("Обновить");
    connect(updateButton, &QPushButton::clicked, this, &ReportWidget::updateReport);
    controlLayout->addWidget(updateButton);
    
    exportButton = new QPushButton("Экспорт в CSV");
    connect(exportButton, &QPushButton::clicked, this, &ReportWidget::exportToCsv);
    controlLayout->addWidget(exportButton);
    
    controlLayout->addStretch();
    mainLayout->addLayout(controlLayout);
    
    // 2. Таблица для отображения отчета
    tableView = new QTableView;
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(tableView);
    
    // 3. Статусная строка (опционально)
    mainLayout->addWidget(new QLabel("Оборотно-сальдовая ведомость"));

    //
    QPushButton *pdfButton = new QPushButton("Экспорт в PDF");
    connect(pdfButton, &QPushButton::clicked, this, &ReportWidget::exportToPdf);
    controlLayout->addWidget(pdfButton);

}

void ReportWidget::setupTable()
{
    model = new QStandardItemModel(this);
    
    // Заголовки столбцов
    QStringList headers;
    headers << "Счет" << "Наименование" 
            << "Начальное Дт" << "Начальное Кт"
            << "Оборот Дт" << "Оборот Кт"
            << "Конечное Дт" << "Конечное Кт";
    
    model->setHorizontalHeaderLabels(headers);
    tableView->setModel(model);
    
    // Настройка ширины столбцов
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->verticalHeader()->setDefaultSectionSize(24);
}

void ReportWidget::updateReport()
{
    QDate startDate = dateStartEdit->date();
    QDate endDate = dateEndEdit->date();
    
    if (startDate > endDate) {
        QMessageBox::warning(this, "Ошибка", "Дата начала не может быть позже даты окончания!");
        return;
    }
    
    // Очищаем таблицу
    model->removeRows(0, model->rowCount());
    
    // Получаем данные из ReportGenerator
    ReportGenerator reportGen;
    QVector<BalanceRecord> report = reportGen.generateBalanceReport(startDate, endDate);
    
    if (report.isEmpty()) {
        QMessageBox::information(this, "Информация", "Нет данных для отображения за выбранный период.");
        return;
    }
    
    // Заполняем таблицу
    for (const BalanceRecord &record : report) {
        QList<QStandardItem*> rowItems;
        
        rowItems << new QStandardItem(record.accountCode);
        rowItems << new QStandardItem(record.accountName);
        rowItems << new QStandardItem(QString::number(record.openingDebit, 'f', 2));
        rowItems << new QStandardItem(QString::number(record.openingCredit, 'f', 2));
        rowItems << new QStandardItem(QString::number(record.turnoverDebit, 'f', 2));
        rowItems << new QStandardItem(QString::number(record.turnoverCredit, 'f', 2));
        rowItems << new QStandardItem(QString::number(record.closingDebit, 'f', 2));
        rowItems << new QStandardItem(QString::number(record.closingCredit, 'f', 2));
        
        // Выделяем итоговые строки (пустые счета)
        if (record.accountCode.isEmpty()) {
            for (QStandardItem *item : rowItems) {
                item->setBackground(QBrush(QColor(240, 240, 240)));
                QFont font = item->font();
                font.setBold(true);
                item->setFont(font);
            }
        }
        
        model->appendRow(rowItems);
    }
}

void ReportWidget::exportToCsv()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт в CSV", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать файл: " + file.errorString());
        return;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    
    // Заголовки
    for (int col = 0; col < model->columnCount(); ++col) {
        stream << "\"" << model->headerData(col, Qt::Horizontal).toString() << "\"";
        if (col < model->columnCount() - 1) stream << ",";
    }
    stream << "\n";
    
    // Данные
    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            QString value = model->index(row, col).data().toString();
            stream << "\"" << value.replace("\"", "\"\"") << "\"";
            if (col < model->columnCount() - 1) stream << ",";
        }
        stream << "\n";
    }
    
    file.close();
    QMessageBox::information(this, "Экспорт", "Данные успешно экспортированы в файл.");
}

void ReportWidget::exportToPdf()
{
    QString defaultName = QString("ОСВ_%1_%2.pdf")
                         .arg(dateStartEdit->date().toString("yyyy-MM-dd"))
                         .arg(dateEndEdit->date().toString("yyyy-MM-dd"));
    
    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт в PDF", 
                                                   defaultName, "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;
    
    // Получаем данные из модели
    QVector<QVariantList> data;
    QStringList headers;
    
    // Заголовки
    for (int col = 0; col < model->columnCount(); ++col) {
        headers << model->headerData(col, Qt::Horizontal).toString();
    }
    
    // Данные
    for (int row = 0; row < model->rowCount(); ++row) {
        QVariantList rowData;
        for (int col = 0; col < model->columnCount(); ++col) {
            rowData << model->index(row, col).data();
        }
        data.append(rowData);
    }
    
    QString title = QString("Оборотно-сальдовая ведомость\n"
                           "За период с %1 по %2")
                   .arg(dateStartEdit->date().toString("dd.MM.yyyy"))
                   .arg(dateEndEdit->date().toString("dd.MM.yyyy"));
    
    if (ExportManager::exportBalanceReportToPdf(data, headers, title, fileName, this)) {
        QMessageBox::information(this, "Экспорт", 
                                "Отчет успешно экспортирован в PDF!");
    }
}
