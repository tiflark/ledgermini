#include <QMetaType>
#include "gui/dialogs/addtransactiondialog.h"
#include "core/database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include "core/validationrules.h"

AddTransactionDialog::AddTransactionDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Добавить проводку");
    setMinimumWidth(500);
    
    setupUI();
    loadAccounts();
    loadCounterparties();
    
    // Изначальная валидация
    validateForm();
}

void AddTransactionDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Форма с полями
    QFormLayout *formLayout = new QFormLayout();
    
    // Дата проводки
    dateEdit = new QDateEdit;
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    formLayout->addRow("Дата проводки *:", dateEdit);
    
    // Дебетовый счет
    debitAccountCombo = new QComboBox;
    debitAccountCombo->setMinimumWidth(300);
    formLayout->addRow("Счет дебета *:", debitAccountCombo);
    
    // Кредитовый счет
    creditAccountCombo = new QComboBox;
    creditAccountCombo->setMinimumWidth(300);
    formLayout->addRow("Счет кредита *:", creditAccountCombo);
    
    // Сумма
    amountSpin = new QDoubleSpinBox;
    amountSpin->setRange(0.01, 1000000000.00);
    amountSpin->setDecimals(2);
    amountSpin->setPrefix("₽ ");
    amountSpin->setSingleStep(100.00);
    formLayout->addRow("Сумма *:", amountSpin);
    
    // Контрагент (необязательно)
    counterpartyCombo = new QComboBox;
    counterpartyCombo->addItem("(не выбран)", QVariant());
    formLayout->addRow("Контрагент:", counterpartyCombo);
    
    // Номер документа
    documentNumberEdit = new QLineEdit;
    formLayout->addRow("Номер документа:", documentNumberEdit);
    
    // Дата документа
    documentDateEdit = new QDateEdit;
    documentDateEdit->setDate(QDate::currentDate());
    documentDateEdit->setCalendarPopup(true);
    formLayout->addRow("Дата документа:", documentDateEdit);
    
    // Описание
    descriptionEdit = new QLineEdit;
    descriptionEdit->setPlaceholderText("Назначение платежа, основание...");
    formLayout->addRow("Описание:", descriptionEdit);
    
    mainLayout->addLayout(formLayout);
    
    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    saveButton = new QPushButton("Сохранить проводку");
    saveButton->setEnabled(false); // Изначально выключена
    QPushButton *cancelButton = new QPushButton("Отмена");
    
    connect(saveButton, &QPushButton::clicked, this, &AddTransactionDialog::saveTransaction);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // Валидация при изменении полей
    connect(debitAccountCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddTransactionDialog::validateForm);
    connect(creditAccountCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddTransactionDialog::validateForm);
    connect(amountSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AddTransactionDialog::validateForm);
    
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
}

void AddTransactionDialog::loadAccounts() {
    if (!Database::instance().isInitialized()) return;
    
    QSqlQuery query = Database::instance().executeQuery(
        "SELECT id, code, name FROM accounts ORDER BY code"
    );
    
    debitAccountCombo->addItem("(выберите счет)", QVariant());
    creditAccountCombo->addItem("(выберите счет)", QVariant());
    
    while (query.next()) {
        int id = query.value(0).toInt();
        QString code = query.value(1).toString();
        QString name = query.value(2).toString();
        QString displayText = QString("%1 - %2").arg(code).arg(name);
        
        debitAccountCombo->addItem(displayText, id);
        creditAccountCombo->addItem(displayText, id);
    }
}

void AddTransactionDialog::loadCounterparties() {
    if (!Database::instance().isInitialized()) return;
    
    QSqlQuery query = Database::instance().executeQuery(
        "SELECT id, name, inn FROM counterparties ORDER BY name"
    );
    
    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        QString inn = query.value(2).toString();
        QString displayText = inn.isEmpty() ? name : QString("%1 (ИНН: %2)").arg(name).arg(inn);
        
        counterpartyCombo->addItem(displayText, id);
    }
}

void AddTransactionDialog::validateForm()
{
    int debitId = debitAccountCombo->currentData().toInt();
    int creditId = creditAccountCombo->currentData().toInt();
    double amount = amountSpin->value();
    QString description = descriptionEdit->text().trimmed();
    
    // Используем ValidationRules
    QDate date = dateEdit->date();
    ValidationRules::TransactionValidation validation = ValidationRules::validateTransaction(date, debitId, creditId, amount, description);

    bool isValid = validation.isValid;
    saveButton->setEnabled(isValid);
    
    // Формируем подсказку
    QString tooltip;
    if (!validation.errorMessage.isEmpty()) {
        tooltip = "<b>Ошибки:</b><br>" + validation.errorMessage.replace("\n", "<br>");
    }
    if (!validation.warningMessage.isEmpty()) {
        if (!tooltip.isEmpty()) tooltip += "<br><br>";
        tooltip += "<b>Предупреждения:</b><br>" + validation.warningMessage.replace("\n", "<br>");
    }
    
    saveButton->setToolTip(tooltip);
    
    // Визуальная индикация
    QString styleSheet = "";
    if (!validation.errorMessage.isEmpty()) {
        styleSheet = "QPushButton { background-color: #ffcccc; border: 1px solid #ff6666; }";
    } else if (!validation.warningMessage.isEmpty()) {
        styleSheet = "QPushButton { background-color: #ffffcc; border: 1px solid #ffcc00; }";
    } else {
        styleSheet = "QPushButton { background-color: #ccffcc; border: 1px solid #66cc66; }";
    }
    
    saveButton->setStyleSheet(styleSheet);
}

void AddTransactionDialog::saveTransaction() {
    // Получаем данные из формы
    QDate transDate = dateEdit->date();
    int debitId = debitAccountCombo->currentData().toInt();
    int creditId = creditAccountCombo->currentData().toInt();
    double amount = amountSpin->value();
    int counterpartyId = counterpartyCombo->currentData().toInt();
    QString description = descriptionEdit->text().trimmed();
    QString docNumber = documentNumberEdit->text().trimmed();
    QDate docDate = documentDateEdit->date();

    // Финальная проверка
    ValidationRules::TransactionValidation validation = 
        ValidationRules::validateTransaction(dateEdit->date(), debitId, creditId, amount, description);
    
    if (!validation.isValid) {
        QMessageBox::critical(this, "Ошибка валидации", 
            "Невозможно сохранить проводку:\n" + validation.errorMessage);
        return;
    }
    
    // Показываем предупреждения, если есть
    if (!validation.warningMessage.isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::warning(this, "Предупреждение",
            validation.warningMessage + "\n\nПродолжить сохранение?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::No) {
            return;
        }
    }
    
    // Проверяем валидность еще раз
    if (debitId == 0 || creditId == 0 || amount <= 0 || debitId == creditId) {
        QMessageBox::warning(this, "Ошибка", "Некорректные данные в форме!");
        return;
    }
    
    // Подготавливаем параметры для запроса
    QVariantList params;
    params << transDate << debitId << creditId << amount;
    params << description << docNumber;
    
    // Дата документа (может быть пустой)
    if (docDate.isValid()) {
    params << docDate;
} else {
    params << QVariant(QMetaType(QMetaType::QDate));
}
    
    // Контрагент (может быть не выбран)
    if (counterpartyId > 0) {
    params << counterpartyId;
} else {
    params << QVariant(QMetaType(QMetaType::Int));
}
    
    // Выполняем запрос
    QString sql = "INSERT INTO transactions ("
                  "transaction_date, debit_account_id, credit_account_id, "
                  "amount, description, document_number, document_date, "
                  "counterparty_id) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
    
    QSqlQuery query = Database::instance().executeQuery(sql, params);
    
    if (query.lastError().isValid()) {
        QMessageBox::critical(this, "Ошибка", 
            "Не удалось сохранить проводку:\n" + query.lastError().text());
    } else {
        QMessageBox::information(this, "Успех", "Проводка успешно добавлена!");
        accept(); // Закрываем диалог с результатом Accepted
    }
}