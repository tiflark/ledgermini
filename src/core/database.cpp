#include "core/database.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

Database& Database::instance()
{
    static Database instance;
    return instance;
}

bool Database::initialize(const std::string& dbPath)
{
    if (initialized_) {
        return true;
    }
    
    db_ = QSqlDatabase::addDatabase("QSQLITE");
    db_.setDatabaseName(QString::fromStdString(dbPath));
    
    if (!db_.open()) {
        qCritical() << "Failed to open database:" << db_.lastError().text();
        return false;
    }
    
    // Включаем поддержку внешних ключей
    QSqlQuery query(db_);
    if (!query.exec("PRAGMA foreign_keys = ON;")) {
        qWarning() << "Failed to enable foreign keys:" << query.lastError().text();
    }
    
    initialized_ = true;
    qInfo() << "Database initialized successfully:" << QString::fromStdString(dbPath);
    return true;
}

bool Database::isInitialized() const
{
    return initialized_;
}

bool Database::executeScript(const std::string& scriptPath)
{
    if (!initialized_) {
        qCritical() << "Database not initialized";
        return false;
    }
    
    QFile file(QString::fromStdString(scriptPath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Failed to open script file:" << QString::fromStdString(scriptPath);
        return false;
    }
    
    QTextStream stream(&file);
    QString sql = stream.readAll();
    file.close();
    
    QStringList statements = sql.split(';', Qt::SkipEmptyParts);
    
    for (const QString& statement : statements) {
        QString trimmed = statement.trimmed();
        if (trimmed.isEmpty()) continue;
        
        QSqlQuery query(db_);
        if (!query.exec(trimmed)) {
            qCritical() << "Failed to execute SQL:" << trimmed
                      << "\nError:" << query.lastError().text();
            return false;
        }
    }
    
    return true;
}

QSqlQuery Database::executeQuery(const QString& queryStr, const QVariantList& params)
{
    QSqlQuery sqlQuery(db_);
    
    // Отладочный вывод
    qDebug() << "\n=== Database::executeQuery ===";
    qDebug() << "SQL:" << queryStr;
    qDebug() << "Expected params:" << params.size();
    
    if (!sqlQuery.prepare(queryStr)) {
        qCritical() << "Failed to prepare query:" << sqlQuery.lastError().text();
        qCritical() << "Query was:" << queryStr;
        return sqlQuery;
    }
    
    // Привязываем параметры
    for (int i = 0; i < params.size(); ++i) {
        sqlQuery.addBindValue(params[i]);
        qDebug() << "  Param" << i << ":" << params[i] << "(" << params[i].typeName() << ")";
    }
    
    if (!sqlQuery.exec()) {
        qCritical() << "Query execution failed:";
        qCritical() << "  Error:" << sqlQuery.lastError().text();
        qCritical() << "  Query:" << sqlQuery.lastQuery();
        qCritical() << "  Bound values:" << sqlQuery.boundValues();
    } else {
        qDebug() << "Query executed successfully, rows affected:" << sqlQuery.numRowsAffected();
    }
    
    return sqlQuery;
}


bool Database::beginTransaction()
{
    return db_.transaction();
}

bool Database::commitTransaction()
{
    return db_.commit();
}

bool Database::rollbackTransaction()
{
    return db_.rollback();
}

QSqlDatabase& Database::database()
{
    return db_;
}