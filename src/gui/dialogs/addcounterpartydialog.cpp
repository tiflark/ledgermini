#include "gui/dialogs/addcounterpartydialog.h"
#include "core/database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>

AddCounterpartyDialog::AddCounterpartyDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Добавить контрагента");
    setupUI();
}

void AddCounterpartyDialog::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    layout->addWidget(new QLabel("Наименование *:"));
    nameEdit = new QLineEdit;
    layout->addWidget(nameEdit);
    
    layout->addWidget(new QLabel("ИНН:"));
    innEdit = new QLineEdit;
    layout->addWidget(innEdit);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *saveBtn = new QPushButton("Сохранить");
    QPushButton *cancelBtn = new QPushButton("Отмена");
    
    connect(saveBtn, &QPushButton::clicked, this, &AddCounterpartyDialog::saveCounterparty);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(cancelBtn);
    layout->addLayout(buttonLayout);
}

void AddCounterpartyDialog::saveCounterparty() {
    if (nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните наименование!");
        return;
    }
    
    // Отладочный вывод
    qDebug() << "Добавление контрагента:";
    qDebug() << "  Название:" << nameEdit->text();
    qDebug() << "  ИНН:" << innEdit->text();
    
    // ПРАВИЛЬНЫЙ SQL-запрос - все 6 полей
    QString sql = "INSERT INTO counterparties (name, inn, kpp, address, phone, email) "
                  "VALUES (?, ?, ?, ?, ?, ?)";
    
    QVariantList params;
    params << nameEdit->text().trimmed()  // name
           << innEdit->text().trimmed()   // inn
           << QString("")                  // kpp
           << QString("")                  // address
           << QString("")                  // phone
           << QString("");                 // email
    
    qDebug() << "SQL:" << sql;
    qDebug() << "Параметры:" << params;
    qDebug() << "Кол-во параметров:" << params.size();
    
    QSqlQuery query = Database::instance().executeQuery(sql, params);
    
    if (query.lastError().isValid()) {
        QString errorMsg = query.lastError().text();
        qDebug() << "Ошибка SQLite:" << errorMsg;
        
        if (errorMsg.contains("UNIQUE", Qt::CaseInsensitive)) {
            QMessageBox::critical(this, "Ошибка", 
                "Контрагент с таким ИНН уже существует!");
        } else {
            QMessageBox::critical(this, "Ошибка", 
                "Не удалось сохранить: " + errorMsg + 
                "\nКол-во параметров: " + QString::number(params.size()));
        }
    } else {
        qDebug() << "Контрагент успешно добавлен!";
        QMessageBox::information(this, "Успех", "Контрагент добавлен!");
        accept();
    }
}
