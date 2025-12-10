#ifndef EXPORTMANAGER_H
#define EXPORTMANAGER_H

#include <QObject>
#include <QTableView>
#include <QString>
#include <QTextDocument>

// Для совместимости Qt5/Qt6
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #include <QPageSize>
    #include <QPrinter>
#else
    #include <QPrinter>
#endif

class ExportManager : public QObject
{
    Q_OBJECT

public:
    explicit ExportManager(QObject *parent = nullptr);
    
    // Экспорт таблицы в PDF
    static bool exportTableToPdf(QTableView *tableView, const QString &title, 
                                const QString &fileName, QWidget *parent = nullptr);
    
    // Экспорт отчета (HTML) в PDF
    static bool exportHtmlToPdf(const QString &htmlContent, const QString &title,
                               const QString &fileName, QWidget *parent = nullptr);
    
    // Экспорт балансового отчета в PDF
    static bool exportBalanceReportToPdf(const QVector<QVariantList> &data,
                                        const QStringList &headers,
                                        const QString &title,
                                        const QString &fileName,
                                        QWidget *parent = nullptr);
    
    // Генерация HTML из таблицы
    static QString generateTableHtml(QTableView *tableView, const QString &title);
    
    // Генерация HTML из данных
    static QString generateDataHtml(const QVector<QVariantList> &data,
                                   const QStringList &headers,
                                   const QString &title);

private:
    static QString getDefaultStyle();
    
    // Объявление с совместимостью Qt5/Qt6
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    static bool saveToPdf(const QString &htmlContent, const QString &fileName,
                         QPageSize::PageSizeId pageSize = QPageSize::A4);
#else
    static bool saveToPdf(const QString &htmlContent, const QString &fileName,
                         QPrinter::PageSize pageSize = QPrinter::A4);
#endif
};

#endif // EXPORTMANAGER_H