#include "gui/dialogs/editcounterpartydialog.h"
#include "core/database.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QLineEdit>           // ДОБАВИТЬ ЭТОТ ЗАГОЛОВОК
#include <QPushButton> 

EditCounterpartyDialog::EditCounterpartyDialog(int counterpartyId, QWidget *parent)
    : QDialog(parent), counterpartyId_(counterpartyId)
{
    setWindowTitle("Редактировать контрагента");
    setMinimumWidth(400);
    
    setupUI();
    loadCounterpartyData();
    validateForm();
}

void EditCounterpartyDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QFormLayout *formLayout = new QFormLayout();
    
    nameEdit_ = new QLineEdit;
    formLayout->addRow("Наименование *:", nameEdit_);
    
    innEdit_ = new QLineEdit;
    formLayout->addRow("ИНН:", innEdit_);
    
    kppEdit_ = new QLineEdit;
    formLayout->addRow("КПП:", kppEdit_);
    
    addressEdit_ = new QLineEdit;
    formLayout->addRow("Адрес:", addressEdit_);
    
    phoneEdit_ = new QLineEdit;
    formLayout->addRow("Телефон:", phoneEdit_);
    
    emailEdit_ = new QLineEdit;
    formLayout->addRow("Email:", emailEdit_);
    
    mainLayout->addLayout(formLayout);
    
    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    saveButton_ = new QPushButton("Сохранить");
    QPushButton *cancelButton = new QPushButton("Отмена");
    
    connect(saveButton_, &QPushButton::clicked, this, &EditCounterpartyDialog::saveCounterparty);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(nameEdit_, &QLineEdit::textChanged, this, &EditCounterpartyDialog::validateForm);
    
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
}

void EditCounterpartyDialog::loadCounterpartyData()
{
    if (!Database::instance().isInitialized()) return;
    
    QSqlQuery query = Database::instance().executeQuery(
        "SELECT name, inn, kpp, address, phone, email "
        "FROM counterparties WHERE id = ?",
        {counterpartyId_}
    );
    
    if (query.next()) {
        nameEdit_->setText(query.value(0).toString());
        innEdit_->setText(query.value(1).toString());
        kppEdit_->setText(query.value(2).toString());
        addressEdit_->setText(query.value(3).toString());
        phoneEdit_->setText(query.value(4).toString());
        emailEdit_->setText(query.value(5).toString());
    }
}

void EditCounterpartyDialog::validateForm()
{
    bool isValid = !nameEdit_->text().trimmed().isEmpty();
    saveButton_->setEnabled(isValid);
}

void EditCounterpartyDialog::saveCounterparty()
{
    QString name = nameEdit_->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните наименование!");
        return;
    }
    
    QString sql = "UPDATE counterparties SET "
                  "name = ?, inn = ?, kpp = ?, address = ?, phone = ?, email = ? "
                  "WHERE id = ?";
    
    QVariantList params;
    params << name
           << innEdit_->text().trimmed()
           << kppEdit_->text().trimmed()
           << addressEdit_->text().trimmed()
           << phoneEdit_->text().trimmed()
           << emailEdit_->text().trimmed()
           << counterpartyId_;
    
    QSqlQuery query = Database::instance().executeQuery(sql, params);
    
    if (query.lastError().isValid()) {
        QMessageBox::critical(this, "Ошибка", 
            "Не удалось обновить контрагента:\n" + query.lastError().text());
    } else {
        QMessageBox::information(this, "Успех", "Контрагент обновлен!");
        accept();
    }
}