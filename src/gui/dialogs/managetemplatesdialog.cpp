#include "gui/dialogs/managetemplatesdialog.h"
#include "gui/dialogs/edittemplatedialog.h"
#include "gui/dialogs/addtransactiondialog.h"
#include "core/database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QInputDialog>

ManageTemplatesDialog::ManageTemplatesDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Управление шаблонами проводок");
    setMinimumSize(800, 500);
    
    setupUI();
    loadTemplates();
}

void ManageTemplatesDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Таблица
    tableView = new QTableView;
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    model = new QStandardItemModel(this);
    tableView->setModel(model);
    
    // Настраиваем заголовки
    QStringList headers = {"ID", "Название", "Описание", "Дебет", "Кредит", 
                          "Сумма", "Частота", "Последнее выполнение"};
    model->setHorizontalHeaderLabels(headers);
    
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->setDefaultSectionSize(24);
    tableView->hideColumn(0); // Скрываем ID
    
    mainLayout->addWidget(tableView);
    
    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    addButton = new QPushButton("Добавить шаблон");
    editButton = new QPushButton("Редактировать");
    deleteButton = new QPushButton("Удалить");
    useButton = new QPushButton("Использовать");
    refreshButton = new QPushButton("Обновить");
    closeButton = new QPushButton("Закрыть");
    
    connect(addButton, &QPushButton::clicked, this, &ManageTemplatesDialog::addTemplate);
    connect(editButton, &QPushButton::clicked, this, &ManageTemplatesDialog::editTemplate);
    connect(deleteButton, &QPushButton::clicked, this, &ManageTemplatesDialog::deleteTemplate);
    connect(useButton, &QPushButton::clicked, this, &ManageTemplatesDialog::useTemplate);
    connect(refreshButton, &QPushButton::clicked, this, &ManageTemplatesDialog::refreshTable);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(tableView, &QTableView::doubleClicked, this, &ManageTemplatesDialog::onDoubleClick);
    
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(useButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    
    mainLayout->addLayout(buttonLayout);
}

void ManageTemplatesDialog::loadTemplates()
{
    model->removeRows(0, model->rowCount());
    
    if (!Database::instance().isInitialized()) return;
    
    QSqlQuery query = Database::instance().executeQuery(
        "SELECT t.id, t.name, t.description, "
        "       d.code || ' - ' || d.name as debit_account, "
        "       c.code || ' - ' || c.name as credit_account, "
        "       t.amount, t.frequency, t.last_executed "
        "FROM transaction_templates t "
        "LEFT JOIN accounts d ON t.debit_account_id = d.id "
        "LEFT JOIN accounts c ON t.credit_account_id = c.id "
        "ORDER BY t.name"
    );
    
    while (query.next()) {
        QList<QStandardItem*> rowItems;
        
        rowItems << new QStandardItem(query.value(0).toString()); // ID
        rowItems << new QStandardItem(query.value(1).toString()); // Название
        rowItems << new QStandardItem(query.value(2).toString()); // Описание
        rowItems << new QStandardItem(query.value(3).toString()); // Дебет
        rowItems << new QStandardItem(query.value(4).toString()); // Кредит
        
        // Сумма
        double amount = query.value(5).toDouble();
        QString amountStr = (amount > 0) ? 
            QString::number(amount, 'f', 2) : "(вводится)";
        rowItems << new QStandardItem(amountStr);
        
        // Частота
        QString freq = query.value(6).toString();
        QString freqText;
        if (freq == "daily") freqText = "Ежедневно";
        else if (freq == "weekly") freqText = "Еженедельно";
        else if (freq == "monthly") freqText = "Ежемесячно";
        else if (freq == "yearly") freqText = "Ежегодно";
        else freqText = "Один раз";
        rowItems << new QStandardItem(freqText);
        
        // Последнее выполнение
        QDate lastExec = query.value(7).toDate();
        rowItems << new QStandardItem(lastExec.isValid() ? 
            lastExec.toString("dd.MM.yyyy") : "Никогда");
        
        model->appendRow(rowItems);
    }
}

void ManageTemplatesDialog::addTemplate()
{
    // TODO: Реализовать добавление шаблона
    QMessageBox::information(this, "Информация", "Функция добавления шаблона будет реализована в следующей версии");
}

void ManageTemplatesDialog::editTemplate()
{
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите шаблон для редактирования");
        return;
    }
    
    int id = model->item(index.row(), 0)->text().toInt();
    // TODO: Реализовать редактирование шаблона
    QMessageBox::information(this, "Информация", 
        QString("Редактирование шаблона ID: %1 будет реализовано в следующей версии").arg(id));
}

void ManageTemplatesDialog::deleteTemplate()
{
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите шаблон для удаления");
        return;
    }
    
    QString templateName = model->item(index.row(), 1)->text();
    int id = model->item(index.row(), 0)->text().toInt();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение удаления",
        QString("Вы уверены, что хотите удалить шаблон '%1'?").arg(templateName),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        // TODO: Реализовать удаление шаблона из базы данных
        QSqlQuery query = Database::instance().executeQuery(
            "DELETE FROM transaction_templates WHERE id = ?",
            {id}
        );
        
        if (query.lastError().isValid()) {
            QMessageBox::critical(this, "Ошибка", 
                "Не удалось удалить шаблон:\n" + query.lastError().text());
        } else {
            loadTemplates();
            QMessageBox::information(this, "Успех", "Шаблон удален");
        }
    }
}

void ManageTemplatesDialog::useTemplate()
{
    QModelIndex index = tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите шаблон для использования");
        return;
    }
    
    int id = model->item(index.row(), 0)->text().toInt();
    QString templateName = model->item(index.row(), 1)->text();
    
    // TODO: Реализовать использование шаблона для создания проводки
    QMessageBox::information(this, "Информация", 
        QString("Использование шаблона '%1' будет реализовано в следующей версии").arg(templateName));
}

void ManageTemplatesDialog::refreshTable()
{
    loadTemplates();
}

void ManageTemplatesDialog::onDoubleClick(const QModelIndex &index)
{
    if (index.isValid()) {
        editTemplate();
    }
}