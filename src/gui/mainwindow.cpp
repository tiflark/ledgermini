#include "gui/mainwindow.h"
#include "gui/dialogs/addtransactiondialog.h"
#include "gui/dialogs/addcounterpartydialog.h"
#include "gui/reportwidget.h"
#include "core/database.h"
#include "gui/tableactions.h"
#include "gui/dialogs/edittransactiondialog.h"
#include "gui/dialogs/editcounterpartydialog.h"
#include "gui/accountcardwidget.h"
#include "gui/dialogs/addeditaccountdialog.h"
#include "gui/operationsjournalwidget.h"
#include "gui/advancedfilterwidget.h"
#include "core/exportmanager.h"  // Добавлено для экспорта в PDF

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSqlRecord>
#include <QDebug>
#include <QSqlQueryModel>  // Добавлено для QSqlQueryModel
#include <QSqlTableModel>  // Добавлено для QSqlTableModel
#include <QSqlError>       // Добавлено для работы с ошибками SQL

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("LedgerMini - Учет по РСБУ"));
    resize(1200, 900);
    
    // Инициализация базы данных
    initializeDatabase();
    
    // Настройка интерфейса
    setupUi();
    setupMenu();
    setupToolbar();
    setupConnections();
    setupTableActions();
    
    // Загрузка данных
    showTransactions();
}

MainWindow::~MainWindow()
{
    // Автоматическое освобождение ресурсов Qt
}

void MainWindow::initializeDatabase()
{
    qDebug() << "=== Инициализация базы данных ===";
    
    // Абсолютный путь к базе данных
    QString dbPath = QDir::homePath() + "/ledgermini/ledgermini.db";
    qDebug() << "Путь к БД:" << dbPath;
    
    // Проверяем существует ли файл базы
    QFile dbFile(dbPath);
    bool dbExists = dbFile.exists();
    qDebug() << "Файл БД существует:" << dbExists;
    
    if (!Database::instance().initialize(dbPath.toStdString())) {
        qCritical() << "ОШИБКА: Не удалось инициализировать базу данных";
        QMessageBox::critical(this, tr("Ошибка"), 
                            tr("Не удалось инициализировать базу данных.\n"
                               "Приложение будет работать в ограниченном режиме."));
    } else {
        qDebug() << "База данных успешно открыта";
        
        // Создаем таблицы, если они не существуют
        createTablesManually();
        
        statusBar()->showMessage(tr("База данных загружена"), 3000);
    }
}

void MainWindow::setupUi()
{
    // Создание центрального виджета с вкладками
    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);
    
    //
    reportWidget = new ReportWidget(this);
    tabWidget->addTab(reportWidget, tr("Отчеты (ОСВ)"));

    //
    accountCardWidget = new AccountCardWidget(this);
    tabWidget->addTab(accountCardWidget, tr("Карточка счета"));


    // Создание таблиц
    transactionsTable = new QTableView;
    accountsTable = new QTableView;
    counterpartiesTable = new QTableView;
    
    // Настройка отображения таблиц
    transactionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    transactionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    transactionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // 1. Создаем виджет поиска для проводок с использованием AdvancedFilterWidget
    transactionsSearchWidget = new AdvancedFilterWidget();

    // 2. Создаем контейнер для проводок с вертикальным layout
    QWidget *transactionsContainer = new QWidget;
    QVBoxLayout *transactionsLayout = new QVBoxLayout(transactionsContainer);

    // 3. Добавляем виджет поиска и таблицу в контейнер
    transactionsLayout->addWidget(transactionsSearchWidget);
    transactionsLayout->addWidget(transactionsTable);

    // 4. Добавляем контейнер на вкладку
    tabWidget->addTab(transactionsContainer, tr("Проводки"));

    // ==================== ДОБАВЛЯЕМ ОСТАЛЬНЫЕ ВКЛАДКИ ====================

    // Создаем контейнеры и для других вкладок для единообразия
    QWidget *accountsContainer = new QWidget;
    QVBoxLayout *accountsLayout = new QVBoxLayout(accountsContainer);
    accountsLayout->addWidget(accountsTable);
    tabWidget->addTab(accountsContainer, tr("План счетов"));

    QWidget *counterpartiesContainer = new QWidget;
    QVBoxLayout *counterpartiesLayout = new QVBoxLayout(counterpartiesContainer);
    counterpartiesLayout->addWidget(counterpartiesTable);
    tabWidget->addTab(counterpartiesContainer, tr("Контрагенты"));

    OperationsJournalWidget *operationsJournal = new OperationsJournalWidget(this);
    tabWidget->addTab(operationsJournal, tr("Журнал операций"));
    
    // Настройка строки состояния
    statusBar()->showMessage(tr("Готово"));
}

 void MainWindow::setupMenu()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&Файл"));
    
    // Добавляем действие для добавления счета
    actionAddAccount = new QAction(tr("&Добавить счет"), this);
    actionAddAccount->setShortcut(QKeySequence("Ctrl+Shift+A"));
    fileMenu->addAction(actionAddAccount);
    
    actionAddTransaction = new QAction(tr("&Добавить проводку"), this);
    actionAddTransaction->setShortcut(QKeySequence::New);
    fileMenu->addAction(actionAddTransaction);
    
    actionAddCounterparty = new QAction(tr("Добавить контрагента"), this);
    fileMenu->addAction(actionAddCounterparty);
    
    fileMenu->addSeparator();
    
    actionBalanceReport = new QAction(tr("&Оборотно-сальдовая ведомость"), this);
    fileMenu->addAction(actionBalanceReport);
    
    fileMenu->addSeparator();
    
    actionExit = new QAction(tr("&Выход"), this);
    actionExit->setShortcut(QKeySequence::Quit);
    fileMenu->addAction(actionExit);
    
    QMenu *helpMenu = menuBar()->addMenu(tr("&Помощь"));
    actionAbout = new QAction(tr("&О программе"), this);
    helpMenu->addAction(actionAbout);

    QMenu *operationsMenu = menuBar()->addMenu(tr("&Операции"));
    QAction *actionManageTemplates = new QAction(tr("Управление шаблонами проводок"), this);
    operationsMenu->addAction(actionManageTemplates);
    
    QAction *actionCreateFromTemplate = new QAction(tr("Создать проводку из шаблона"), this);
    operationsMenu->addAction(actionCreateFromTemplate);
    
    // Подключаем сигналы
    connect(actionAddAccount, &QAction::triggered, this, &MainWindow::addAccount);

    // 4. Добавляем журнал операций
    OperationsJournalWidget *operationsJournal = new OperationsJournalWidget(this);
    tabWidget->addTab(operationsJournal, tr("Журнал операций"));
    
    // Настраиваем политику размеров для tabWidget
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Устанавливаем центральный виджет с отступами
    QWidget *central = new QWidget;
    QVBoxLayout *centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(5, 5, 5, 5);
    centralLayout->addWidget(tabWidget);
    setCentralWidget(central);
    
    // Настройка строки состояния
    statusBar()->showMessage(tr("Готово"));
}

void MainWindow::setupToolbar()
{
    QToolBar *mainToolbar = addToolBar(tr("Основные действия"));
    mainToolbar->addAction(actionAddAccount);
    mainToolbar->addAction(actionAddTransaction);
    mainToolbar->addAction(actionBalanceReport);
    mainToolbar->addSeparator();
    mainToolbar->addAction(actionAddCounterparty);

    
    QToolBar *navigationToolbar = addToolBar(tr("Навигация"));
    navigationToolbar->addAction(tr("Проводки"), this, &MainWindow::showTransactions);
    navigationToolbar->addAction(tr("Счета"), this, &MainWindow::showAccounts);
    navigationToolbar->addAction(tr("Контрагенты"), this, &MainWindow::showCounterparties);
    
}

void MainWindow::setupConnections()
{
    // Подключение действий меню
    connect(actionAddAccount, &QAction::triggered, this, &MainWindow::addAccount);
    connect(actionAddTransaction, &QAction::triggered, this, &MainWindow::addTransaction);
    connect(actionAddCounterparty, &QAction::triggered, this, &MainWindow::addCounterparty);
    connect(actionBalanceReport, &QAction::triggered, this, &MainWindow::generateBalanceReport);
    connect(actionExit, &QAction::triggered, qApp, &QApplication::quit);
    connect(actionAbout, &QAction::triggered, this, &MainWindow::about);
    
    // Подключение поиска проводок через AdvancedFilterWidget
    if (transactionsSearchWidget) {
        connect(transactionsSearchWidget, &AdvancedFilterWidget::filterApplied,
                this, &MainWindow::onSearchTransactions);
    }
    
    // Подключение смены вкладок
    connect(tabWidget, &QTabWidget::currentChanged, [this](int index) {
        switch (index) {
            case 0: showTransactions(); break;
            case 1: showAccounts(); break;
            case 2: showCounterparties(); break;
        }
    });
}

void MainWindow::showTransactions() {
    if (!Database::instance().isInitialized()) return;
    
    // Получаем параметры фильтра из AdvancedFilterWidget
    AdvancedFilterWidget::FilterOptions options;
    if (transactionsSearchWidget) {
        options = transactionsSearchWidget->getFilterOptions();
    }
    
    // Если есть активный фильтр, применяем его
    if (!options.textFilter.isEmpty() || 
        options.dateFilterEnabled || 
        options.amountFilterEnabled ||
        options.debitAccountId != -1 ||
        options.creditAccountId != -1 ||
        options.counterpartyId != -1) {
        
        onSearchTransactions();
    } else {
        // Иначе показываем все проводки
        QSqlQueryModel *model = new QSqlQueryModel(this);
        QString query = 
            "SELECT t.id, t.transaction_date, "
            "       d.code || ' - ' || d.name as debit, "
            "       c.code || ' - ' || c.name as credit, "
            "       t.amount, t.description, t.document_number, "
            "       COALESCE(cp.name, '') as counterparty_name "
            "FROM transactions t "
            "LEFT JOIN accounts d ON t.debit_account_id = d.id "
            "LEFT JOIN accounts c ON t.credit_account_id = c.id "
            "LEFT JOIN counterparties cp ON t.counterparty_id = cp.id "
            "ORDER BY t.transaction_date DESC, t.id DESC";
        
        model->setQuery(query);
        model->setHeaderData(0, Qt::Horizontal, tr("ID"));
        model->setHeaderData(1, Qt::Horizontal, tr("Дата"));
        model->setHeaderData(2, Qt::Horizontal, tr("Дебет"));
        model->setHeaderData(3, Qt::Horizontal, tr("Кредит"));
        model->setHeaderData(4, Qt::Horizontal, tr("Сумма"));
        model->setHeaderData(5, Qt::Horizontal, tr("Описание"));
        model->setHeaderData(6, Qt::Horizontal, tr("Документ"));
        model->setHeaderData(7, Qt::Horizontal, tr("Контрагент"));
        
        // Устанавливаем модель
        QSqlQueryModel *oldModel = qobject_cast<QSqlQueryModel*>(transactionsTable->model());
        transactionsTable->setModel(model);
        
        if (oldModel) {
            oldModel->deleteLater();
        }
        
        transactionsTable->hideColumn(0);
        
        // Настраиваем адаптивные размеры столбцов
        transactionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        transactionsTable->setColumnWidth(1, 80);   // Дата
        transactionsTable->setColumnWidth(2, 150);  // Дебет
        transactionsTable->setColumnWidth(3, 150);  // Кредит
        transactionsTable->setColumnWidth(4, 80);   // Сумма
        transactionsTable->setColumnWidth(5, 200);  // Описание (растягивается)
        transactionsTable->setColumnWidth(6, 100);  // Документ
        transactionsTable->setColumnWidth(7, 150);  // Контрагент
        
        // Столбец "Описание" растягивается
        transactionsTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
        
        // Показываем количество записей
        int rowCount = model->rowCount();
        statusBar()->showMessage(QString("Всего проводок: %1").arg(rowCount), 3000);
    }
}

void MainWindow::showAccounts()
{
    if (!Database::instance().isInitialized()) return;
    
    // Используем QSqlQueryModel с иерархическим запросом
    QSqlQueryModel *model = new QSqlQueryModel(this);
    
    // Рекурсивный запрос для иерархического отображения
    QString query = 
        "WITH RECURSIVE account_tree AS ("
        "  SELECT id, code, name, type, parent_id, 0 as level, "
        "         code as sort_key "
        "  FROM accounts WHERE parent_id IS NULL "
        "  UNION ALL "
        "  SELECT a.id, a.code, a.name, a.type, a.parent_id, at.level + 1, "
        "         at.sort_key || '.' || a.code "
        "  FROM accounts a "
        "  JOIN account_tree at ON a.parent_id = at.id"
        ") "
        "SELECT id, "
        "       printf('%*s%s', level * 2, '', code) as code_display, "
        "       name, "
        "       CASE type "
        "         WHEN 0 THEN 'Активный' "
        "         WHEN 1 THEN 'Пассивный' "
        "         WHEN 2 THEN 'Активно-пассивный' "
        "       END as type_name "
        "FROM account_tree "
        "ORDER BY sort_key";
    
    model->setQuery(query);
    
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Код"));
    model->setHeaderData(2, Qt::Horizontal, tr("Наименование"));
    model->setHeaderData(3, Qt::Horizontal, tr("Тип"));
    
    accountsTable->setModel(model);
    accountsTable->hideColumn(0); // Скрываем ID
    
    // Настройка отображения
    accountsTable->setAlternatingRowColors(true);
    accountsTable->verticalHeader()->setDefaultSectionSize(24);
    accountsTable->horizontalHeader()->setStretchLastSection(true);
    
    // Настраиваем выравнивание для кода
    accountsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
}

void MainWindow::showCounterparties()
{
    if (!Database::instance().isInitialized()) return;
    
    counterpartiesModel = new QSqlTableModel(this, Database::instance().database());
    counterpartiesModel->setTable("counterparties");
    
    counterpartiesModel->setHeaderData(1, Qt::Horizontal, tr("Наименование"));
    counterpartiesModel->setHeaderData(2, Qt::Horizontal, tr("ИНН"));
    counterpartiesModel->setHeaderData(3, Qt::Horizontal, tr("КПП"));
    
    counterpartiesModel->select();
    counterpartiesTable->setModel(counterpartiesModel);
    counterpartiesTable->hideColumn(0); // Скрываем ID
}

void MainWindow::addTransaction() {
    AddTransactionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        showTransactions(); // Обновляем таблицу проводок
        statusBar()->showMessage("Проводка добавлена", 3000);
    }
}

void MainWindow::generateBalanceReport()
{
    // Находим индекс вкладки с отчетами
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabText(i).contains("Отчеты") || 
            tabWidget->tabText(i).contains("ОСВ")) {
            tabWidget->setCurrentIndex(i);
            
            // Если виджет отчета имеет метод updateReport(), вызываем его
            QWidget *currentWidget = tabWidget->widget(i);
            if (ReportWidget *reportWidget = qobject_cast<ReportWidget*>(currentWidget)) {
                reportWidget->updateReport();
            }
            
            statusBar()->showMessage("Открыта оборотно-сальдовая ведомость", 3000);
            return;
        }
    }
    
    // Если вкладка не найдена, показываем информационное сообщение
    QMessageBox::information(this, tr("Информация"),
                            tr("Вкладка с отчетами уже открыта."));
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("О программе LedgerMini"),
        tr("<h2>LedgerMini 0.1.0</h2>"
           "<p>Простая система учета для формирования отчетности по РСБУ.</p>"
           "<p>Разработано для демонстрации навыков C++/Qt разработки.</p>"
           "<p><b>Текущие возможности:</b></p>"
           "<ul>"
           "<li>Просмотр плана счетов РСБУ</li>"
           "<li>Просмотр списка контрагентов</li>"
           "<li>Просмотр бухгалтерских проводок</li>"
           "</ul>"
           "<p>Используемые технологии: Qt6, SQLite, CMake</p>"));
}

void MainWindow::addCounterparty() {
    AddCounterpartyDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        showCounterparties(); // Обновляем таблицу
        statusBar()->showMessage("Контрагент добавлен", 3000);
    }
}

void MainWindow::setupTableActions()
{
    // Действия для таблицы проводок
    transactionsActions = new TableActions(transactionsTable, this);
    connect(transactionsActions, &TableActions::editRequested,
            this, &MainWindow::editTransaction);
    connect(transactionsActions, &TableActions::deleteRequested,
            this, &MainWindow::deleteTransaction);
    connect(transactionsActions, &TableActions::refreshRequested,
            this, &MainWindow::showTransactions);
    
    // Действия для таблицы контрагентов
    counterpartiesActions = new TableActions(counterpartiesTable, this);
    connect(counterpartiesActions, &TableActions::editRequested,
            this, &MainWindow::editCounterparty);
    connect(counterpartiesActions, &TableActions::deleteRequested,
            this, &MainWindow::deleteCounterparty);
    connect(counterpartiesActions, &TableActions::refreshRequested,
            this, &MainWindow::showCounterparties);
    
    // Действия для таблицы счетов
    accountsActions = new TableActions(accountsTable, this);
    connect(accountsActions, &TableActions::addRequested,
            this, &MainWindow::addAccount);
    connect(accountsActions, &TableActions::editRequested,
            this, &MainWindow::editAccount);
    connect(accountsActions, &TableActions::deleteRequested,
            this, &MainWindow::deleteAccount);
    connect(accountsActions, &TableActions::refreshRequested,
            this, &MainWindow::showAccounts);

    //
    connect(transactionsActions, &TableActions::exportToPdfRequested,
            this, &MainWindow::exportTransactionsToPdf);
    connect(accountsActions, &TableActions::exportToPdfRequested,
            this, &MainWindow::exportAccountsToPdf);
    connect(counterpartiesActions, &TableActions::exportToPdfRequested,
            this, &MainWindow::exportCounterpartiesToPdf);
}

void MainWindow::addAccount()
{
    AddEditAccountDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        showAccounts();
        statusBar()->showMessage("Счет добавлен", 3000);
    }
}

void MainWindow::editAccount(int id)
{
    AddEditAccountDialog dialog(id, this);
    if (dialog.exec() == QDialog::Accepted) {
        showAccounts();
        statusBar()->showMessage("Счет обновлен", 3000);
    }
}

void MainWindow::editTransaction(int id)
{
    EditTransactionDialog dialog(id, this);
    if (dialog.exec() == QDialog::Accepted) {
        showTransactions();
        statusBar()->showMessage("Проводка обновлена", 3000);
    }
}

void MainWindow::deleteTransaction(int id)
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение удаления",
        "Вы уверены, что хотите удалить эту проводку?",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QSqlQuery query = Database::instance().executeQuery(
            "DELETE FROM transactions WHERE id = ?",
            {id}
        );
        
        if (query.lastError().isValid()) {
            QMessageBox::critical(this, "Ошибка",
                "Не удалось удалить проводку:\n" + query.lastError().text());
        } else {
            showTransactions();
            statusBar()->showMessage("Проводка удалена", 3000);
        }
    }
}

void MainWindow::editCounterparty(int id)
{
    EditCounterpartyDialog dialog(id, this);
    if (dialog.exec() == QDialog::Accepted) {
        showCounterparties();
        statusBar()->showMessage("Контрагент обновлен", 3000);
    }
}

void MainWindow::deleteCounterparty(int id)
{
    // Проверяем, есть ли связанные проводки
    QSqlQuery checkQuery = Database::instance().executeQuery(
        "SELECT COUNT(*) FROM transactions WHERE counterparty_id = ?",
        {id}
    );
    
    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        QMessageBox::warning(this, "Невозможно удалить",
            "Контрагент используется в проводках. Сначала удалите связанные проводки.");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение удаления",
        "Вы уверены, что хотите удалить этого контрагента?",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QSqlQuery query = Database::instance().executeQuery(
            "DELETE FROM counterparties WHERE id = ?",
            {id}
        );
        
        if (query.lastError().isValid()) {
            QMessageBox::critical(this, "Ошибка",
                "Не удалось удалить контрагента:\n" + query.lastError().text());
        } else {
            showCounterparties();
            statusBar()->showMessage("Контрагент удален", 3000);
        }
    }
}

void MainWindow::deleteAccount(int id)
{
    // Проверяем, есть ли связанные проводки
    QSqlQuery checkQuery = Database::instance().executeQuery(
        "SELECT COUNT(*) FROM transactions WHERE debit_account_id = ? OR credit_account_id = ?",
        {id, id}
    );
    
    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        QMessageBox::warning(this, "Невозможно удалить",
            "Счет используется в проводках. Сначала удалите связанные проводки.");
        return;
    }
    
    // Проверяем, есть ли дочерние счета
    checkQuery = Database::instance().executeQuery(
        "SELECT COUNT(*) FROM accounts WHERE parent_id = ?",
        {id}
    );
    
    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        QMessageBox::warning(this, "Невозможно удалить",
            "Счет имеет дочерние счета. Сначала удалите или переместите дочерние счета.");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение удаления",
        "Вы уверены, что хотите удалить этот счет?",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QSqlQuery query = Database::instance().executeQuery(
            "DELETE FROM accounts WHERE id = ?",
            {id}
        );
        
        if (query.lastError().isValid()) {
            QMessageBox::critical(this, "Ошибка",
                "Не удалось удалить счет:\n" + query.lastError().text());
        } else {
            showAccounts();
            statusBar()->showMessage("Счет удален", 3000);
        }
    }
}

void MainWindow::refreshTable()
{
    int currentTab = tabWidget->currentIndex();
    switch (currentTab) {
        case 0: showTransactions(); break;
        case 1: showAccounts(); break;
        case 2: showCounterparties(); break;
    }
}

void MainWindow::checkDatabaseTables()
{
    qDebug() << "=== Проверка таблиц БД ===";
    
    QStringList tables = {"accounts", "counterparties", "transactions"};
    
    for (const QString &table : tables) {
        QSqlQuery query = Database::instance().executeQuery(
            "SELECT name FROM sqlite_master WHERE type='table' AND name=?",
            {table}
        );
        
        if (query.next()) {
            qDebug() << "✓ Таблица" << table << "существует";
        } else {
            qWarning() << "✗ Таблица" << table << "НЕ существует!";
            // Если таблицы нет, создаем ее
            createTablesManually();
            break;
        }
    }
}

void MainWindow::createTablesManually()
{
    qDebug() << "=== Создание таблиц ===";
    
    // Создаем таблицу контрагентов
    QString createCounterparties = 
        "CREATE TABLE IF NOT EXISTS counterparties ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    name TEXT NOT NULL,"
        "    inn TEXT UNIQUE,"
        "    kpp TEXT DEFAULT '',"
        "    address TEXT DEFAULT '',"
        "    phone TEXT DEFAULT '',"
        "    email TEXT DEFAULT '',"
        "    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";
    
    QSqlQuery query1 = Database::instance().executeQuery(createCounterparties);
    if (query1.lastError().isValid()) {
        qCritical() << "Ошибка создания таблицы counterparties:" << query1.lastError().text();
    } else {
        qDebug() << "✓ Таблица counterparties проверена/создана";
    }
    
    // Создаем таблицу счетов
    QString createAccounts = 
        "CREATE TABLE IF NOT EXISTS accounts ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    code TEXT NOT NULL UNIQUE,"
        "    name TEXT NOT NULL,"
        "    type INTEGER NOT NULL,"
        "    parent_id INTEGER,"
        "    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "    FOREIGN KEY (parent_id) REFERENCES accounts(id) ON DELETE CASCADE"
        ")";
    
    QSqlQuery query2 = Database::instance().executeQuery(createAccounts);
    if (query2.lastError().isValid()) {
        qCritical() << "Ошибка создания таблицы accounts:" << query2.lastError().text();
    } else {
        qDebug() << "✓ Таблица accounts проверена/создана";
        
        // Вставляем базовые счета, если их нет
        QString insertAccounts = 
            "INSERT OR IGNORE INTO accounts (code, name, type) VALUES "
            "('50', 'Касса', 0),"
            "('51', 'Расчетные счета', 0),"
            "('52', 'Валютные счета', 0),"
            "('60', 'Расчеты с поставщиками и подрядчиками', 2),"
            "('62', 'Расчеты с покупателями и заказчиками', 2),"
            "('70', 'Расчеты с персоналом по оплате труда', 2),"
            "('80', 'Уставный капитал', 1),"
            "('90', 'Продажи', 1),"
            "('91', 'Прочие доходы и расходы', 2)";
        
        QSqlQuery query3 = Database::instance().executeQuery(insertAccounts);
        if (query3.lastError().isValid()) {
            qWarning() << "Ошибка вставки базовых счетов:" << query3.lastError().text();
        } else {
            qDebug() << "✓ Базовые счета проверены/добавлены";
        }
    }
    
    // Создаем таблицу проводок
    QString createTransactions = 
        "CREATE TABLE IF NOT EXISTS transactions ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    transaction_date DATE NOT NULL,"
        "    debit_account_id INTEGER NOT NULL,"
        "    credit_account_id INTEGER NOT NULL,"
        "    amount DECIMAL(15,2) NOT NULL,"
        "    description TEXT,"
        "    document_number TEXT,"
        "    document_date DATE,"
        "    counterparty_id INTEGER,"
        "    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "    FOREIGN KEY (debit_account_id) REFERENCES accounts(id),"
        "    FOREIGN KEY (credit_account_id) REFERENCES accounts(id),"
        "    FOREIGN KEY (counterparty_id) REFERENCES counterparties(id),"
        "    CHECK (amount > 0)"
        ")";
    
    QSqlQuery query4 = Database::instance().executeQuery(createTransactions);
    if (query4.lastError().isValid()) {
        qCritical() << "Ошибка создания таблицы transactions:" << query4.lastError().text();
    } else {
        qDebug() << "✓ Таблица transactions проверена/создана";
    }
    
    // Создаем индексы, если не существуют
    QStringList indexes = {
        "CREATE INDEX IF NOT EXISTS idx_transactions_date ON transactions(transaction_date)",
        "CREATE INDEX IF NOT EXISTS idx_transactions_debit ON transactions(debit_account_id)",
        "CREATE INDEX IF NOT EXISTS idx_transactions_credit ON transactions(credit_account_id)"
    };
    
    for (const QString &index : indexes) {
        QSqlQuery query = Database::instance().executeQuery(index);
        if (query.lastError().isValid()) {
            qWarning() << "Ошибка создания индекса:" << query.lastError().text();
        }
    }
    
    qDebug() << "=== Все таблицы проверены/созданы ===";
}

void MainWindow::onSearchTransactions()
{
    if (!Database::instance().isInitialized()) return;
    
    // Получаем параметры фильтрации из AdvancedFilterWidget
    AdvancedFilterWidget::FilterOptions options;
    if (transactionsSearchWidget) {
        options = transactionsSearchWidget->getFilterOptions();
    }
    
    // Строим базовый запрос
    QString query = 
        "SELECT t.id, t.transaction_date, "
        "       d.code || ' - ' || d.name as debit, "
        "       c.code || ' - ' || c.name as credit, "
        "       t.amount, t.description, t.document_number, "
        "       COALESCE(cp.name, '') as counterparty_name "
        "FROM transactions t "
        "LEFT JOIN accounts d ON t.debit_account_id = d.id "
        "LEFT JOIN accounts c ON t.credit_account_id = c.id "
        "LEFT JOIN counterparties cp ON t.counterparty_id = cp.id "
        "WHERE 1=1";  // Всегда истинное условие для удобства добавления других
    
    // Добавляем фильтр по дате, если включен
    if (options.dateFilterEnabled) {
        query += " AND t.transaction_date BETWEEN '" + options.dateFrom.toString("yyyy-MM-dd") + 
                 "' AND '" + options.dateTo.toString("yyyy-MM-dd") + "'";
    }
    
    // Добавляем фильтр по сумме, если включен
    if (options.amountFilterEnabled) {
        query += " AND t.amount BETWEEN " + QString::number(options.amountFrom) +
                 " AND " + QString::number(options.amountTo);
    }
    
    // Добавляем фильтр по счету дебета, если выбран
    if (options.debitAccountId != -1) {
        query += " AND t.debit_account_id = " + QString::number(options.debitAccountId);
    }
    
    // Добавляем фильтр по счету кредита, если выбран
    if (options.creditAccountId != -1) {
        query += " AND t.credit_account_id = " + QString::number(options.creditAccountId);
    }
    
    // Добавляем фильтр по контрагенту, если выбран
    if (options.counterpartyId != -1) {
        query += " AND t.counterparty_id = " + QString::number(options.counterpartyId);
    }
    
    // Добавляем текстовый фильтр, если есть
    if (!options.textFilter.isEmpty()) {
        QString likePattern = "%" + options.textFilter + "%";
        
        if (options.fieldFilter.isEmpty() || options.fieldFilter == "all") {
            // Ищем во всех полях
            query += " AND ("
                     "  d.code LIKE '" + likePattern + "' OR "
                     "  d.name LIKE '" + likePattern + "' OR "
                     "  c.code LIKE '" + likePattern + "' OR "
                     "  c.name LIKE '" + likePattern + "' OR "
                     "  t.description LIKE '" + likePattern + "' OR "
                     "  t.document_number LIKE '" + likePattern + "' OR "
                     "  cp.name LIKE '" + likePattern + "'"
                     ")";
        } else if (options.fieldFilter == "description") {
            query += " AND t.description LIKE '" + likePattern + "'";
        } else if (options.fieldFilter == "comment") {
            query += " AND (t.description LIKE '" + likePattern + "' OR t.document_number LIKE '" + likePattern + "')";
        }
    }
    
    query += " ORDER BY t.transaction_date DESC, t.id DESC";
    
    // Выполняем запрос
    QSqlQueryModel *model = new QSqlQueryModel(this);
    model->setQuery(query);
    
    if (model->lastError().isValid()) {
        qWarning() << "Ошибка выполнения поискового запроса:" << model->lastError().text();
        delete model;
        return;
    }
    
    // Устанавливаем заголовки
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Дата"));
    model->setHeaderData(2, Qt::Horizontal, tr("Дебет"));
    model->setHeaderData(3, Qt::Horizontal, tr("Кредит"));
    model->setHeaderData(4, Qt::Horizontal, tr("Сумма"));
    model->setHeaderData(5, Qt::Horizontal, tr("Описание"));
    model->setHeaderData(6, Qt::Horizontal, tr("Документ"));
    model->setHeaderData(7, Qt::Horizontal, tr("Контрагент"));
    
    // Устанавливаем модель в таблицу
    QSqlQueryModel *oldModel = qobject_cast<QSqlQueryModel*>(transactionsTable->model());
    transactionsTable->setModel(model);
    
    // Удаляем старую модель
    if (oldModel) {
        oldModel->deleteLater();
    }
    
    // Настраиваем отображение
    transactionsTable->hideColumn(0);  // Скрываем ID
    transactionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // Показываем количество найденных записей
    int rowCount = model->rowCount();
    QString message = QString("Найдено проводок: %1").arg(rowCount);
    
    if (!options.textFilter.isEmpty()) {
        message += QString(" по запросу: \"%1\"").arg(options.textFilter);
    }
    
    statusBar()->showMessage(message, 5000);
}

void MainWindow::exportTransactionsToPdf()
{
    QString defaultName = QString("Проводки_%1.pdf")
                         .arg(QDate::currentDate().toString("yyyy-MM-dd"));
    
    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт проводок в PDF",
                                                   defaultName, "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;
    
    QString title = "Журнал проводок\n"
                   "Сформировано: " + QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm");
    
    if (ExportManager::exportTableToPdf(transactionsTable, title, fileName, this)) {
        statusBar()->showMessage("Проводки экспортированы в PDF", 3000);
    }
}

void MainWindow::exportAccountsToPdf()
{
    QString defaultName = QString("План_счетов_%1.pdf")
                         .arg(QDate::currentDate().toString("yyyy-MM-dd"));
    
    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт плана счетов в PDF",
                                                   defaultName, "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;
    
    QString title = "План счетов РСБУ\n"
                   "Сформировано: " + QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm");
    
    if (ExportManager::exportTableToPdf(accountsTable, title, fileName, this)) {
        statusBar()->showMessage("План счетов экспортирован в PDF", 3000);
    }
}

void MainWindow::exportCounterpartiesToPdf()
{
    QString defaultName = QString("Контрагенты_%1.pdf")
                         .arg(QDate::currentDate().toString("yyyy-MM-dd"));
    
    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт контрагентов в PDF",
                                                   defaultName, "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;
    
    QString title = "Справочник контрагентов\n"
                   "Сформировано: " + QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm");
    
    if (ExportManager::exportTableToPdf(counterpartiesTable, title, fileName, this)) {
        statusBar()->showMessage("Контрагенты экспортированы в PDF", 3000);
    }
}