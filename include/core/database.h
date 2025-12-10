#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <memory>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class Database
{
public:
    static Database& instance();
    
    bool initialize(const std::string& dbPath = "ledgermini.db");
    bool isInitialized() const;
    
    bool executeScript(const std::string& scriptPath);
    QSqlQuery executeQuery(const QString& query, const QVariantList& params = {});
    
    // Для работы с транзакциями
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // Добавленный метод для получения базы данных
    QSqlDatabase& database();

private:
    Database() = default;
    ~Database() = default;
    
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    QSqlDatabase db_;
    bool initialized_ = false;
};

#endif // DATABASE_H
