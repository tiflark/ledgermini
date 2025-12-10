#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H
#include <QObject>
#include <QDate>
#include <QVector>
#include <QString>

struct BalanceRecord {
    QString accountCode;
    QString accountName;
    int accountType = 0;          // 0=Активный, 1=Пассивный, 2=Активно-пассивный
    double openingDebit = 0.0;    // Начальное сальдо по дебету
    double openingCredit = 0.0;   // Начальное сальдо по кредиту
    double turnoverDebit = 0.0;   // Оборот по дебету
    double turnoverCredit = 0.0;  // Оборот по кредиту
    double closingDebit = 0.0;    // Конечное сальдо по дебету
    double closingCredit = 0.0;   // Конечное сальдо по кредиту
    
    // Методы для удобства
    double openingBalance() const { return openingDebit - openingCredit; }
    double closingBalance() const { return closingDebit - closingCredit; }
    double totalTurnover() const { return turnoverDebit + turnoverCredit; }
};

class ReportGenerator : public QObject {
    Q_OBJECT
public:
    explicit ReportGenerator(QObject *parent = nullptr);
    
    // Основные отчеты
    QVector<BalanceRecord> generateBalanceReport(const QDate &startDate, const QDate &endDate);
    
    // Дополнительные отчеты
    QVector<QVector<QVariant>> generateAccountAnalysis(int accountId, const QDate &startDate, const QDate &endDate);
    QVector<QVector<QVariant>> generateCounterpartyReport(int counterpartyId, const QDate &startDate, const QDate &endDate);
    
    // Вспомогательные методы
    double calculateAccountBalance(int accountId, const QDate &date);
    double calculateAccountTurnover(int accountId, const QDate &startDate, const QDate &endDate, bool isDebit);
    
private:
    void calculateFinalBalances(QVector<BalanceRecord> &records);
};

#endif