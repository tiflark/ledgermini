#include "core/validationrules.h"
#include <QRegularExpression>
#include <QDate>

ValidationRules::ValidationRules(QObject *parent)
    : QObject(parent)
{
}

ValidationRules::TransactionValidation ValidationRules::validateTransaction(
    const QDate &date,
    int debitAccountId,
    int creditAccountId,
    double amount,
    const QString &description)
{
    TransactionValidation validation;
    
    if (!validateDate(date)) {
        validation.isValid = false;
        validation.errorMessage = "Неверная дата транзакции";
        return validation;
    }
    
    if (debitAccountId <= 0) {
        validation.isValid = false;
        validation.errorMessage = "Не выбран счет дебета";
        return validation;
    }
    
    if (creditAccountId <= 0) {
        validation.isValid = false;
        validation.errorMessage = "Не выбран счет кредита";
        return validation;
    }
    
    if (debitAccountId == creditAccountId) {
        validation.isValid = false;
        validation.errorMessage = "Счета дебета и кредита не могут совпадать";
        return validation;
    }
    
    if (!validateAmount(amount)) {
        validation.isValid = false;
        validation.errorMessage = "Неверная сумма транзакции";
        return validation;
    }
    
    if (description.trimmed().isEmpty()) {
        validation.warningMessage = "Описание транзакции не заполнено";
    }
    
    return validation;
}

bool ValidationRules::validateAccount(const QString &code, const QString &name, int type)
{
    if (code.trimmed().isEmpty()) {
        return false;
    }
    
    if (!isValidAccountCode(code)) {
        return false;
    }
    
    if (name.trimmed().isEmpty()) {
        return false;
    }
    
    if (type < 0 || type > 2) {
        return false;
    }
    
    return true;
}

bool ValidationRules::validateCounterparty(const QString &name, const QString &inn)
{
    if (name.trimmed().isEmpty()) {
        return false;
    }
    
    if (!inn.trimmed().isEmpty() && !isValidINN(inn)) {
        return false;
    }
    
    return true;
}

bool ValidationRules::validateAmount(double amount)
{
    return amount > 0 && amount <= 1000000000; // Ограничение на максимальную сумму
}

bool ValidationRules::validateDate(const QDate &date)
{
    return date.isValid() && date <= QDate::currentDate();
}

bool ValidationRules::isValidAccountCode(const QString &code)
{
    // Код счета должен состоять только из цифр и точек
    QRegularExpression regex("^[0-9.]*$");
    return regex.match(code).hasMatch() && code.length() <= 20;
}

bool ValidationRules::isValidINN(const QString &inn)
{
    // Проверка ИНН (10 или 12 цифр)
    QString cleanInn = inn.trimmed();
    QRegularExpression regex("^[0-9]{10}$|^[0-9]{12}$");
    return regex.match(cleanInn).hasMatch();
}
