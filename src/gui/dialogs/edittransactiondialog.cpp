#include "gui/dialogs/edittransactiondialog.h"
#include "core/database.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QDateEdit>         // ДОБАВИТЬ
#include <QComboBox>         // ДОБАВИТЬ  
#include <QLineEdit>         // ДОБАВИТЬ
#include <QDoubleSpinBox>    // ДОБАВИТЬ
#include <QDate> 


EditTransactionDialog::EditTransactionDialog(int transactionId, QWidget *parent)
    : AddTransactionDialog(parent), transactionId_(transactionId)
{
    setWindowTitle("Редактировать проводку");
    loadTransactionData();
}

void EditTransactionDialog::loadTransactionData()
{
    if (!Database::instance().isInitialized()) return;
    
    QSqlQuery query = Database::instance().executeQuery(
        "SELECT t.transaction_date, t.debit_account_id, t.credit_account_id, "
        "t.amount, t.description, t.document_number, t.document_date, "
        "t.counterparty_id "
        "FROM transactions t WHERE t.id = ?",
        {transactionId_}
    );
    
    if (query.next()) {
        // Устанавливаем значения в форму
        dateEdit->setDate(query.value(0).toDate());
        
        // Находим индексы в комбобоксах
        int debitId = query.value(1).toInt();
        int creditId = query.value(2).toInt();
        
        for (int i = 0; i < debitAccountCombo->count(); i++) {
            if (debitAccountCombo->itemData(i).toInt() == debitId) {
                debitAccountCombo->setCurrentIndex(i);
                break;
            }
        }
        
        for (int i = 0; i < creditAccountCombo->count(); i++) {
            if (creditAccountCombo->itemData(i).toInt() == creditId) {
                creditAccountCombo->setCurrentIndex(i);
                break;
            }
        }
        
        amountSpin->setValue(query.value(3).toDouble());
        descriptionEdit->setText(query.value(4).toString());
        documentNumberEdit->setText(query.value(5).toString());
        
        if (!query.value(6).isNull()) {
            documentDateEdit->setDate(query.value(6).toDate());
        }
        
        int counterpartyId = query.value(7).toInt();
        if (counterpartyId > 0) {
            for (int i = 0; i < counterpartyCombo->count(); i++) {
                if (counterpartyCombo->itemData(i).toInt() == counterpartyId) {
                    counterpartyCombo->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
}

void EditTransactionDialog::saveTransaction()
{
    // Получаем данные из формы (родительский метод)
    QDate transDate = dateEdit->date();
    int debitId = debitAccountCombo->currentData().toInt();
    int creditId = creditAccountCombo->currentData().toInt();
    double amount = amountSpin->value();
    int counterpartyId = counterpartyCombo->currentData().toInt();
    QString description = descriptionEdit->text().trimmed();
    QString docNumber = documentNumberEdit->text().trimmed();
    QDate docDate = documentDateEdit->date();
    
    // Проверяем валидность
    if (debitId == 0 || creditId == 0 || amount <= 0 || debitId == creditId) {
        QMessageBox::warning(this, "Ошибка", "Некорректные данные в форме!");
        return;
    }
    
    // Подготавливаем параметры для UPDATE запроса
    QVariantList params;
    params << transDate << debitId << creditId << amount;
    params << description << docNumber;
    
    if (docDate.isValid()) {
    params << docDate;
} else {
    params << QVariant(QMetaType(QMetaType::QDate));
}
    
    if (counterpartyId > 0) {
    params << counterpartyId;
} else {
    params << QVariant(QMetaType(QMetaType::Int));
}
    
    params << transactionId_; // WHERE id = ?
    
    // Выполняем UPDATE запрос
    QString sql = "UPDATE transactions SET "
                  "transaction_date = ?, debit_account_id = ?, credit_account_id = ?, "
                  "amount = ?, description = ?, document_number = ?, document_date = ?, "
                  "counterparty_id = ? "
                  "WHERE id = ?";
    
    QSqlQuery query = Database::instance().executeQuery(sql, params);
    
    if (query.lastError().isValid()) {
        QMessageBox::critical(this, "Ошибка", 
            "Не удалось обновить проводку:\n" + query.lastError().text());
    } else {
        QMessageBox::information(this, "Успех", "Проводка обновлена!");
        accept();
    }
}