#include "gui/reportwidget.h"
#include "core/report_generator.h"
#include "core/database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDebug>
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
#include <QBrush>
#include <QFont>
#include <QDebug>

ReportGenerator::ReportGenerator(QObject *parent) : QObject(parent) {}

QVector<BalanceRecord> ReportGenerator::generateBalanceReport(const QDate &startDate, const QDate &endDate)
{
    QVector<BalanceRecord> report;
    
    // Проверяем валидность дат
    if (startDate > endDate) {
        qWarning() << "Дата начала позже даты окончания";
        return report;
    }
    
    // Получаем все счета
    QSqlQuery accountsQuery = Database::instance().executeQuery(
        "SELECT id, code, name, type FROM accounts ORDER BY code"
    );
    
    if (!accountsQuery.isActive()) {
        qWarning() << "Не удалось получить список счетов";
        return report;
    }
    
    double totalOpeningDebit = 0.0;
    double totalOpeningCredit = 0.0;
    double totalTurnoverDebit = 0.0;
    double totalTurnoverCredit = 0.0;
    double totalClosingDebit = 0.0;
    double totalClosingCredit = 0.0;
    
    while (accountsQuery.next()) {
        int accountId = accountsQuery.value(0).toInt();
        BalanceRecord record;
        record.accountCode = accountsQuery.value(1).toString();
        record.accountName = accountsQuery.value(2).toString();
        record.accountType = accountsQuery.value(3).toInt();
        
        qDebug() << "Обработка счета:" << record.accountCode << record.accountName;
        
        // 1. Рассчитываем начальное сальдо (на дату startDate - 1 день)
        QDate openingDate = startDate.addDays(-1);
        QSqlQuery openingQuery = Database::instance().executeQuery(
            "SELECT "
            "  SUM(CASE WHEN debit_account_id = ? THEN amount ELSE 0 END) as debit_sum, "
            "  SUM(CASE WHEN credit_account_id = ? THEN amount ELSE 0 END) as credit_sum "
            "FROM transactions "
            "WHERE transaction_date <= ?",
            {accountId, accountId, openingDate}
        );
        
        if (openingQuery.next()) {
            double totalDebit = openingQuery.value(0).toDouble();
            double totalCredit = openingQuery.value(1).toDouble();
            
            // Для активных счетов: сальдо начальное по дебету
            // Для пассивных: по кредиту
            // Для активно-пассивных: зависит от разницы
            double openingBalance = totalDebit - totalCredit;
            
            if (record.accountType == 0) { // Активный
                if (openingBalance >= 0) {
                    record.openingDebit = openingBalance;
                } else {
                    record.openingCredit = -openingBalance;
                }
            } else if (record.accountType == 1) { // Пассивный
                if (openingBalance <= 0) {
                    record.openingCredit = -openingBalance;
                } else {
                    record.openingDebit = openingBalance;
                }
            } else { // Активно-пассивный
                if (openingBalance >= 0) {
                    record.openingDebit = openingBalance;
                } else {
                    record.openingCredit = -openingBalance;
                }
            }
        }
        
        // 2. Рассчитываем обороты за период
        QSqlQuery turnoverQuery = Database::instance().executeQuery(
            "SELECT "
            "  SUM(CASE WHEN debit_account_id = ? THEN amount ELSE 0 END) as debit_turnover, "
            "  SUM(CASE WHEN credit_account_id = ? THEN amount ELSE 0 END) as credit_turnover "
            "FROM transactions "
            "WHERE transaction_date BETWEEN ? AND ?",
            {accountId, accountId, startDate, endDate}
        );
        
        if (turnoverQuery.next()) {
            record.turnoverDebit = turnoverQuery.value(0).toDouble();
            record.turnoverCredit = turnoverQuery.value(1).toDouble();
        }
        
        // 3. Рассчитываем конечное сальдо
        // Для активных счетов: Конечное = НачальноеДебет + ОборотДебет - ОборотКредит
        // Для пассивных: Конечное = НачальноеКредит + ОборотКредит - ОборотДебет
        if (record.accountType == 0) { // Активный
            double closingBalance = record.openingDebit + record.turnoverDebit - record.turnoverCredit;
            if (closingBalance >= 0) {
                record.closingDebit = closingBalance;
            } else {
                record.closingCredit = -closingBalance;
            }
        } else if (record.accountType == 1) { // Пассивный
            double closingBalance = record.openingCredit + record.turnoverCredit - record.turnoverDebit;
            if (closingBalance >= 0) {
                record.closingCredit = closingBalance;
            } else {
                record.closingDebit = -closingBalance;
            }
        } else { // Активно-пассивный
            double closingBalance = (record.openingDebit - record.openingCredit) + 
                                   (record.turnoverDebit - record.turnoverCredit);
            if (closingBalance >= 0) {
                record.closingDebit = closingBalance;
            } else {
                record.closingCredit = -closingBalance;
            }
        }
        
        // Суммируем для итоговой строки
        totalOpeningDebit += record.openingDebit;
        totalOpeningCredit += record.openingCredit;
        totalTurnoverDebit += record.turnoverDebit;
        totalTurnoverCredit += record.turnoverCredit;
        totalClosingDebit += record.closingDebit;
        totalClosingCredit += record.closingCredit;
        
        report.append(record);
    }
    
    // Добавляем итоговую строку
    if (!report.isEmpty()) {
        BalanceRecord totalRecord;
        totalRecord.accountCode = "";
        totalRecord.accountName = "ИТОГО:";
        totalRecord.openingDebit = totalOpeningDebit;
        totalRecord.openingCredit = totalOpeningCredit;
        totalRecord.turnoverDebit = totalTurnoverDebit;
        totalRecord.turnoverCredit = totalTurnoverCredit;
        totalRecord.closingDebit = totalClosingDebit;
        totalRecord.closingCredit = totalClosingCredit;
        
        report.append(totalRecord);
    }
    
    return report;
}

double ReportGenerator::calculateAccountBalance(int accountId, const QDate &date)
{
    QSqlQuery query = Database::instance().executeQuery(
        "SELECT "
        "  SUM(CASE WHEN debit_account_id = ? THEN amount ELSE 0 END) - "
        "  SUM(CASE WHEN credit_account_id = ? THEN amount ELSE 0 END) as balance "
        "FROM transactions "
        "WHERE transaction_date <= ?",
        {accountId, accountId, date}
    );
    
    if (query.next()) {
        return query.value(0).toDouble();
    }
    
    return 0.0;
}

double ReportGenerator::calculateAccountTurnover(int accountId, const QDate &startDate, 
                                               const QDate &endDate, bool isDebit)
{
    QString field = isDebit ? "debit_account_id" : "credit_account_id";
    
    QSqlQuery query = Database::instance().executeQuery(
        QString("SELECT SUM(amount) FROM transactions "
                "WHERE %1 = ? AND transaction_date BETWEEN ? AND ?").arg(field),
        {accountId, startDate, endDate}
    );
    
    if (query.next()) {
        return query.value(0).toDouble();
    }
    
    return 0.0;
}