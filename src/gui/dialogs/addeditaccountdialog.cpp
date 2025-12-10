#include "gui/dialogs/addeditaccountdialog.h"
#include "core/database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QRegularExpressionValidator>  // ЗАМЕНА: вместо QRegExpValidator
#include <QRegularExpression>           // ДОБАВЛЯЕМ

AddEditAccountDialog::AddEditAccountDialog(QWidget *parent)
    : QDialog(parent), isEditMode_(false)
{
    setWindowTitle("Добавить счет");
    setupUI();
    loadParentAccounts();
    validateForm();
}

AddEditAccountDialog::AddEditAccountDialog(int accountId, QWidget *parent)
    : QDialog(parent), accountId_(accountId), isEditMode_(true)
{
    setWindowTitle("Редактировать счет");
    setupUI();
    loadParentAccounts();
    loadAccountData();
    validateForm();
}

void AddEditAccountDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QFormLayout *formLayout = new QFormLayout();
    
    // Код счета
    codeEdit_ = new QLineEdit;
    codeEdit_->setPlaceholderText("Например: 50, 51.01, 60.1");
    
    // Валидатор для кода счета (цифры, точки, тире)
    // Qt6: используем QRegularExpression вместо QRegExp
    QRegularExpression codeRegex("^[0-9]+([\\.\\-][0-9]+)*$");
    QRegularExpressionValidator *codeValidator = new QRegularExpressionValidator(codeRegex, this);
    codeEdit_->setValidator(codeValidator);
    
    formLayout->addRow("Код счета *:", codeEdit_);
    
    // Метка для отображения валидации кода
    codeValidationLabel_ = new QLabel;
    codeValidationLabel_->setStyleSheet("color: red; font-size: 10px;");
    formLayout->addRow("", codeValidationLabel_);
    
    // Наименование счета
    nameEdit_ = new QLineEdit;
    formLayout->addRow("Наименование *:", nameEdit_);
    
    // Тип счета
    typeCombo_ = new QComboBox;
    typeCombo_->addItem("Активный", 0);
    typeCombo_->addItem("Пассивный", 1);
    typeCombo_->addItem("Активно-пассивный", 2);
    formLayout->addRow("Тип счета *:", typeCombo_);
    
    // Родительский счет
    parentCombo_ = new QComboBox;
    parentCombo_->addItem("(без родителя)", QVariant());
    formLayout->addRow("Родительский счет:", parentCombo_);
    
    // Описание
    descriptionEdit_ = new QTextEdit;
    descriptionEdit_->setMaximumHeight(100);
    formLayout->addRow("Описание:", descriptionEdit_);
    
    mainLayout->addLayout(formLayout);
    
    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    saveButton_ = new QPushButton(isEditMode_ ? "Сохранить изменения" : "Добавить счет");
    QPushButton *cancelButton = new QPushButton("Отмена");
    
    connect(saveButton_, &QPushButton::clicked, this, &AddEditAccountDialog::saveAccount);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(codeEdit_, &QLineEdit::textChanged, this, &AddEditAccountDialog::validateForm);
    connect(nameEdit_, &QLineEdit::textChanged, this, &AddEditAccountDialog::validateForm);
    
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
}

void AddEditAccountDialog::loadParentAccounts()
{
    if (!Database::instance().isInitialized()) return;
    
    QSqlQuery query = Database::instance().executeQuery(
        "SELECT id, code, name FROM accounts WHERE parent_id IS NULL ORDER BY code"
    );
    
    while (query.next()) {
        int id = query.value(0).toInt();
        QString code = query.value(1).toString();
        QString name = query.value(2).toString();
        QString displayText = QString("%1 - %2").arg(code).arg(name);
        
        parentCombo_->addItem(displayText, id);
    }
}

void AddEditAccountDialog::loadAccountData()
{
    if (!Database::instance().isInitialized() || accountId_ <= 0) return;
    
    QSqlQuery query = Database::instance().executeQuery(
        "SELECT code, name, type, parent_id, "
        "(SELECT code FROM accounts p WHERE p.id = a.parent_id) as parent_code "
        "FROM accounts a WHERE id = ?",
        {accountId_}
    );
    
    if (query.next()) {
        codeEdit_->setText(query.value(0).toString());
        nameEdit_->setText(query.value(1).toString());
        
        int type = query.value(2).toInt();
        for (int i = 0; i < typeCombo_->count(); i++) {
            if (typeCombo_->itemData(i).toInt() == type) {
                typeCombo_->setCurrentIndex(i);
                break;
            }
        }
        
        int parentId = query.value(3).toInt();
        if (parentId > 0) {
            QString parentCode = query.value(4).toString();
            // Находим родительский счет в комбобоксе
            for (int i = 0; i < parentCombo_->count(); i++) {
                if (parentCombo_->itemData(i).toInt() == parentId) {
                    parentCombo_->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
}

void AddEditAccountDialog::validateForm()
{
    QString code = codeEdit_->text().trimmed();
    QString name = nameEdit_->text().trimmed();
    
    bool isValid = true;
    QStringList errors;
    
    // Проверка кода
    if (code.isEmpty()) {
        isValid = false;
        errors << "Код счета не может быть пустым";
        codeValidationLabel_->setText("");
    } else {
        // Проверка формата кода - используем QRegularExpression
        QRegularExpression codeRegex("^[0-9]+([\\.\\-][0-9]+)*$");
        if (!codeRegex.match(code).hasMatch()) {  // Используем match() вместо exactMatch()
            isValid = false;
            errors << "Код счета может содержать только цифры, точки и тире";
            codeValidationLabel_->setText("Только цифры, точки (.) и тире (-)");
        } else {
            codeValidationLabel_->setText("");
        }
        
        // Проверка на уникальность кода (только при добавлении)
        if (!isEditMode_ && Database::instance().isInitialized()) {
            QSqlQuery checkQuery = Database::instance().executeQuery(
                "SELECT COUNT(*) FROM accounts WHERE code = ?",
                {code}
            );
            
            if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
                isValid = false;
                errors << "Счет с таким кодом уже существует";
                codeValidationLabel_->setText("Счет с таким кодом уже существует");
            }
        }
    }
    
    // Проверка наименования
    if (name.isEmpty()) {
        isValid = false;
        errors << "Наименование счета не может быть пустым";
    }
    
    // Проверка: счет не может быть родителем самому себе
    if (isEditMode_ && accountId_ > 0) {
        int parentId = parentCombo_->currentData().toInt();
        if (parentId == accountId_) {
            isValid = false;
            errors << "Счет не может быть родителем самому себе";
        }
    }
    
    saveButton_->setEnabled(isValid);
    
    if (!isValid) {
        saveButton_->setToolTip(errors.join("\n"));
    } else {
        saveButton_->setToolTip("");
    }
}

void AddEditAccountDialog::saveAccount()
{
    QString code = codeEdit_->text().trimmed();
    QString name = nameEdit_->text().trimmed();
    int type = typeCombo_->currentData().toInt();
    int parentId = parentCombo_->currentData().toInt();
    QString description = descriptionEdit_->toPlainText().trimmed();
    
    // Дополнительная валидация
    if (code.isEmpty() || name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните обязательные поля!");
        return;
    }
    
    QVariantList params;
    QString sql;
    
    if (isEditMode_) {
        // Редактирование существующего счета
        sql = "UPDATE accounts SET code = ?, name = ?, type = ?, parent_id = ? WHERE id = ?";
        params << code << name << type;
        
        if (parentId > 0) {
            params << parentId;
        } else {
            params << QVariant();
        }
        
        params << accountId_;
    } else {
        // Добавление нового счета
        sql = "INSERT INTO accounts (code, name, type, parent_id) VALUES (?, ?, ?, ?)";
        params << code << name << type;
        
        if (parentId > 0) {
            params << parentId;
        } else {
            params << QVariant();
        }
    }
    
    QSqlQuery query = Database::instance().executeQuery(sql, params);
    
    if (query.lastError().isValid()) {
        QString errorMsg = query.lastError().text();
        if (errorMsg.contains("UNIQUE")) {
            QMessageBox::critical(this, "Ошибка", 
                "Счет с таким кодом уже существует!");
        } else if (errorMsg.contains("FOREIGN KEY")) {
            QMessageBox::critical(this, "Ошибка", 
                "Некорректный родительский счет!");
        } else {
            QMessageBox::critical(this, "Ошибка", 
                "Не удалось сохранить счет:\n" + errorMsg);
        }
    } else {
        QMessageBox::information(this, "Успех", 
            isEditMode_ ? "Счет обновлен!" : "Счет добавлен!");
        accept();
    }
}