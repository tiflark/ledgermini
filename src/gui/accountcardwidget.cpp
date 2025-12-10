#include "gui/accountcardwidget.h"
#include "core/database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>           // ДОБАВИТЬ
#include <QDateEdit>           // ДОБАВИТЬ
#include <QTableView>          // ДОБАВИТЬ
#include <QPushButton>         // ДОБАВИТЬ
#include <QStandardItemModel>  // ДОБАВИТЬ
#include <QHeaderView>         // ДОБАВИТЬ
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QSqlQuery>

AccountCardWidget::AccountCardWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupTable();
    loadAccounts();
    
    // Устанавливаем период по умолчанию (текущий месяц)
    QDate today = QDate::currentDate();
    dateStartEdit->setDate(QDate(today.year(), today.month(), 1));
    dateEndEdit->setDate(today);
    
    // Обновляем отчет при запуске
    updateReport();
}

void AccountCardWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Панель управления
    QHBoxLayout *controlLayout = new QHBoxLayout();
    
    controlLayout->addWidget(new QLabel("Счет:"));
    accountCombo = new QComboBox;
    accountCombo->setMinimumWidth(300);
    controlLayout->addWidget(accountCombo);
    
    controlLayout->addWidget(new QLabel("Период с:"));
    dateStartEdit = new QDateEdit;
    dateStartEdit->setCalendarPopup(true);
    controlLayout->addWidget(dateStartEdit);
    
    controlLayout->addWidget(new QLabel("по:"));
    dateEndEdit = new QDateEdit;
    dateEndEdit->setCalendarPopup(true);
    controlLayout->addWidget(dateEndEdit);
    
    updateButton = new QPushButton("Обновить");
    connect(updateButton, &QPushButton::clicked, this, &AccountCardWidget::updateReport);
    controlLayout->addWidget(updateButton);
    
    exportButton = new QPushButton("Экспорт");
    connect(exportButton, &QPushButton::clicked, this, &AccountCardWidget::exportToCsv);
    controlLayout->addWidget(exportButton);
    
    controlLayout->addStretch();
    mainLayout->addLayout(controlLayout);
    
    // Таблица
    tableView = new QTableView;
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(tableView);
    
    mainLayout->addWidget(new QLabel("Карточка счета - детализация проводок"));
}

void AccountCardWidget::setupTable()
{
    model = new QStandardItemModel(this);
    
    QStringList headers;
    headers << "Дата" << "Документ" << "Контрагент" 
            << "Дебет" << "Кредит" << "Сумма" << "Описание";
    
    model->setHorizontalHeaderLabels(headers);
    tableView->setModel(model);
    
    // Настройка ширины столбцов
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->setDefaultSectionSize(24);
}

void AccountCardWidget::loadAccounts()
{
    if (!Database::instance().isInitialized()) return;
    
    accountCombo->clear();
    accountCombo->addItem("(выберите счет)", QVariant());
    
    QSqlQuery query = Database::instance().executeQuery(
        "SELECT id, code, name FROM accounts ORDER BY code"
    );
    
    while (query.next()) {
        int id = query.value(0).toInt();
        QString code = query.value(1).toString();
        QString name = query.value(2).toString();
        QString displayText = QString("%1 - %2").arg(code).arg(name);
        
        accountCombo->addItem(displayText, id);
    }
}

void AccountCardWidget::updateReport()
{
    int accountId = accountCombo->currentData().toInt();
    QDate startDate = dateStartEdit->date();
    QDate endDate = dateEndEdit->date();
    
    if (accountId == 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите счет!");
        return;
    }
    
    if (startDate > endDate) {
        QMessageBox::warning(this, "Ошибка", "Дата начала не может быть позже даты окончания!");
        return;
    }
    
    // Очищаем таблицу
    model->removeRows(0, model->rowCount());
    
    // Получаем данные о счете
    QSqlQuery accountQuery = Database::instance().executeQuery(
        "SELECT code, name FROM accounts WHERE id = ?",
        {accountId}
    );
    
    QString accountCode, accountName;
    if (accountQuery.next()) {
        accountCode = accountQuery.value(0).toString();
        accountName = accountQuery.value(1).toString();
    }
    
    // Получаем проводки по счету
    QString sql = 
        "SELECT t.transaction_date, t.document_number, "
        "       COALESCE(cp.name, '') as counterparty_name, "
        "       CASE WHEN t.debit_account_id = ? THEN 'Дт' ELSE 'Кт' END as direction, "
        "       a2.code as opposite_account_code, "
        "       a2.name as opposite_account_name, "
        "       t.amount, t.description "
        "FROM transactions t "
        "LEFT JOIN accounts a1 ON (t.debit_account_id = a1.id OR t.credit_account_id = a1.id) "
        "LEFT JOIN accounts a2 ON "
        "   (CASE WHEN t.debit_account_id = ? THEN t.credit_account_id ELSE t.debit_account_id END) = a2.id "
        "LEFT JOIN counterparties cp ON t.counterparty_id = cp.id "
        "WHERE (t.debit_account_id = ? OR t.credit_account_id = ?) "
        "  AND t.transaction_date BETWEEN ? AND ? "
        "ORDER BY t.transaction_date, t.id";
    
    QVariantList params = {accountId, accountId, accountId, accountId, startDate, endDate};
    QSqlQuery query = Database::instance().executeQuery(sql, params);
    
    double totalDebit = 0.0;
    double totalCredit = 0.0;
    
    while (query.next()) {
        QList<QStandardItem*> rowItems;
        
        // Дата
        rowItems << new QStandardItem(query.value(0).toDate().toString("dd.MM.yyyy"));
        
        // Документ
        rowItems << new QStandardItem(query.value(1).toString());
        
        // Контрагент
        rowItems << new QStandardItem(query.value(2).toString());
        
        // Дебет/Кредит
        QString direction = query.value(3).toString();
        QString oppositeAccount = QString("%1 - %2")
            .arg(query.value(4).toString())
            .arg(query.value(5).toString());
        
        if (direction == "Дт") {
            rowItems << new QStandardItem(oppositeAccount);
            rowItems << new QStandardItem("");
            totalDebit += query.value(6).toDouble();
        } else {
            rowItems << new QStandardItem("");
            rowItems << new QStandardItem(oppositeAccount);
            totalCredit += query.value(6).toDouble();
        }
        
        // Сумма
        rowItems << new QStandardItem(QString::number(query.value(6).toDouble(), 'f', 2));
        
        // Описание
        rowItems << new QStandardItem(query.value(7).toString());
        
        model->appendRow(rowItems);
    }
    
    // Добавляем итоговую строку
    if (model->rowCount() > 0) {
        model->appendRow({
            new QStandardItem("ИТОГО:"),
            new QStandardItem(""),
            new QStandardItem(""),
            new QStandardItem(QString::number(totalDebit, 'f', 2)),
            new QStandardItem(QString::number(totalCredit, 'f', 2)),
            new QStandardItem(""),
            new QStandardItem("")
        });
        
        // Выделяем итоговую строку
        int lastRow = model->rowCount() - 1;
        for (int col = 0; col < model->columnCount(); ++col) {
            QStandardItem *item = model->item(lastRow, col);
            item->setBackground(QBrush(QColor(240, 240, 240)));
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
        }
    }
    
    // Обновляем заголовок
    tableView->setWindowTitle(QString("Карточка счета %1 - %2")
        .arg(accountCode).arg(accountName));
}

void AccountCardWidget::exportToCsv()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Экспорт в CSV", "", "CSV Files (*.csv)");
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", 
            "Не удалось создать файл: " + file.errorString());
        return;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    
    // Заголовки
    for (int col = 0; col < model->columnCount(); ++col) {
        stream << "\"" << model->headerData(col, Qt::Horizontal).toString() << "\"";
        if (col < model->columnCount() - 1) stream << ";";
    }
    stream << "\n";
    
    // Данные
    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            QString value = model->index(row, col).data().toString();
            stream << "\"" << value.replace("\"", "\"\"") << "\"";
            if (col < model->columnCount() - 1) stream << ";";
        }
        stream << "\n";
    }
    
    file.close();
    QMessageBox::information(this, "Экспорт", "Данные успешно экспортированы.");
}