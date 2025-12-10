#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QTabWidget;
class QTableView;
class QSqlTableModel;
class ReportWidget;
class TableActions;
class AccountCardWidget;
class AdvancedFilterWidget;
class OperationsJournalWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void about();
    void showTransactions();
    void showAccounts();
    void showCounterparties();
    void addTransaction();
    void addCounterparty();
    void generateBalanceReport();
    
    // Новые слоты для редактирования/удаления
    void editTransaction(int id);
    void deleteTransaction(int id);
    void editCounterparty(int id);
    void deleteCounterparty(int id);
    void editAccount(int id);
    void deleteAccount(int id);
    void refreshTable();
    
    // Новый слот для добавления счета
    void addAccount();

    void exportTransactionsToPdf();
    void exportAccountsToPdf();
    void exportCounterpartiesToPdf();
    void onSearchTransactions();

private:
    void setupUi();
    void setupMenu();
    void setupToolbar();
    void setupConnections();
    void initializeDatabase();
    void setupTableActions();

    // Новые приватные методы для работы с БД
    void checkDatabaseTables();
    void createTablesManually();

    // Виджеты
    QTabWidget *tabWidget;
    QTableView *transactionsTable;
    QTableView *accountsTable;
    QTableView *counterpartiesTable;
    ReportWidget *reportWidget;
    AccountCardWidget *accountCardWidget;
    AdvancedFilterWidget *transactionsSearchWidget;
    OperationsJournalWidget *operationsJournalWidget;

    // Модели данных
    QSqlTableModel *transactionsModel;
    QSqlTableModel *accountsModel;
    QSqlTableModel *counterpartiesModel;

    // Действия для контекстных меню
    TableActions *transactionsActions;
    TableActions *accountsActions;
    TableActions *counterpartiesActions;

    // Действия меню
    QAction *actionAddAccount;
    QAction *actionAddTransaction;
    QAction *actionAddCounterparty;
    QAction *actionBalanceReport;
    QAction *actionExit;
    QAction *actionAbout;
};

#endif // MAINWINDOW_H