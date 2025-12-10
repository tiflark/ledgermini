#ifndef VALIDATIONRULES_H
#define VALIDATIONRULES_H

#include <QObject>
#include <QString>
#include <QDate>

class ValidationRules : public QObject
{
    Q_OBJECT

public:
    struct TransactionValidation {
        bool isValid;
        QString errorMessage;
        QString warningMessage;
        
        TransactionValidation() : isValid(true) {}
    };

    explicit ValidationRules(QObject *parent = nullptr);

    // Проверка проводки
    static TransactionValidation validateTransaction(
        const QDate &date,
        int debitAccountId,
        int creditAccountId,
        double amount,
        const QString &description
    );

    // Проверка счета
    static bool validateAccount(const QString &code, const QString &name, int type);

    // Проверка контрагента
    static bool validateCounterparty(const QString &name, const QString &inn);

    // Проверка суммы
    static bool validateAmount(double amount);

    // Проверка даты
    static bool validateDate(const QDate &date);

private:
    static bool isValidAccountCode(const QString &code);
    static bool isValidINN(const QString &inn);
};

#endif // VALIDATIONRULES_H
