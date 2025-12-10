#include "core/exportmanager.h"
#include <QTableView>
#include <QHeaderView>
#include <QPrinter>
#include <QTextDocument>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QTextStream>
#include <QTextTable>
#include <QTextCursor>
#include <QTextTableFormat>
#include <QTextCharFormat>
#include <QTextBlockFormat>

ExportManager::ExportManager(QObject *parent) : QObject(parent) {}

bool ExportManager::exportTableToPdf(QTableView *tableView, const QString &title,
                                   const QString &fileName, QWidget *parent)
{
    if (!tableView || !tableView->model()) {
        QMessageBox::warning(parent, "Ошибка", "Нет данных для экспорта");
        return false;
    }
    
    QString html = generateTableHtml(tableView, title);
    return saveToPdf(html, fileName);
}

bool ExportManager::exportHtmlToPdf(const QString &htmlContent, const QString &title,
                                  const QString &fileName, QWidget *parent)
{
    QString fullHtml = "<html><head><meta charset='UTF-8'>"
                      "<style>" + getDefaultStyle() + "</style></head><body>";
    fullHtml += "<h1>" + title + "</h1>";
    fullHtml += htmlContent;
    fullHtml += "</body></html>";
    
    return saveToPdf(fullHtml, fileName);
}

bool ExportManager::exportBalanceReportToPdf(const QVector<QVariantList> &data,
                                           const QStringList &headers,
                                           const QString &title,
                                           const QString &fileName,
                                           QWidget *parent)
{
    QString html = generateDataHtml(data, headers, title);
    return saveToPdf(html, fileName);
}

QString ExportManager::generateTableHtml(QTableView *tableView, const QString &title)
{
    QString html;
    QTextStream stream(&html);
    
    stream << "<div class='report'>";
    stream << "<h1>" << title << "</h1>";
    
    QAbstractItemModel *model = tableView->model();
    if (!model) return "";
    
    int rowCount = model->rowCount();
    int colCount = model->columnCount();
    
    // Определяем видимые столбцы
    QVector<int> visibleColumns;
    for (int col = 0; col < colCount; ++col) {
        if (!tableView->isColumnHidden(col)) {
            visibleColumns.append(col);
        }
    }
    
    if (visibleColumns.isEmpty()) return "";
    
    stream << "<table class='data-table'>";
    
    // Заголовки
    stream << "<thead><tr>";
    for (int col : visibleColumns) {
        QString header = model->headerData(col, Qt::Horizontal).toString();
        stream << "<th>" << header << "</th>";
    }
    stream << "</tr></thead>";
    
    // Данные
    stream << "<tbody>";
    for (int row = 0; row < rowCount; ++row) {
        QString rowClass = (row % 2 == 0) ? "even" : "odd";
        
        // Проверяем, является ли строка итоговой
        QModelIndex firstIndex = model->index(row, 0);
        if (firstIndex.isValid()) {
            QString firstCell = model->data(firstIndex).toString();
            if (firstCell.isEmpty() || firstCell.contains("ИТОГО")) {
                rowClass = "total";
            }
        }
        
        stream << "<tr class='" << rowClass << "'>";
        for (int col : visibleColumns) {
            QModelIndex index = model->index(row, col);
            QString value = model->data(index).toString();
            
            // Выравнивание для числовых значений
            QString align = "left";
            if (col >= 2) { // Предполагаем, что числовые данные с третьего столбца
                align = "right";
            }
            
            stream << "<td style='text-align:" << align << "'>" << value << "</td>";
        }
        stream << "</tr>";
    }
    stream << "</tbody>";
    
    stream << "</table>";
    stream << "</div>";
    
    return html;
}

QString ExportManager::generateDataHtml(const QVector<QVariantList> &data,
                                      const QStringList &headers,
                                      const QString &title)
{
    QString html;
    QTextStream stream(&html);
    
    stream << "<div class='report'>";
    stream << "<h1>" << title << "</h1>";
    
    if (headers.isEmpty() || data.isEmpty()) {
        stream << "<p>Нет данных для отображения</p>";
        stream << "</div>";
        return html;
    }
    
    stream << "<table class='data-table'>";
    
    // Заголовки
    stream << "<thead><tr>";
    for (const QString &header : headers) {
        stream << "<th>" << header << "</th>";
    }
    stream << "</tr></thead>";
    
    // Данные
    stream << "<tbody>";
    for (int row = 0; row < data.size(); ++row) {
        QString rowClass = (row % 2 == 0) ? "even" : "odd";
        
        // Проверяем, является ли строка итоговой
        if (!data[row].isEmpty() && 
            (data[row][0].toString().isEmpty() || data[row][0].toString().contains("ИТОГО"))) {
            rowClass = "total";
        }
        
        stream << "<tr class='" << rowClass << "'>";
        for (int col = 0; col < data[row].size(); ++col) {
            QString value = data[row][col].toString();
            
            // Выравнивание для числовых значений (предполагаем с третьего столбца)
            QString align = (col >= 2) ? "right" : "left";
            
            stream << "<td style='text-align:" << align << "'>" << value << "</td>";
        }
        stream << "</tr>";
    }
    stream << "</tbody>";
    
    stream << "</table>";
    stream << "</div>";
    
    return html;
}

QString ExportManager::getDefaultStyle()
{
    return R"(
        body {
            font-family: 'DejaVu Sans', 'Arial', sans-serif;
            font-size: 10pt;
            margin: 20px;
        }
        h1 {
            text-align: center;
            font-size: 14pt;
            margin-bottom: 20px;
            color: #333;
        }
        .report {
            width: 100%;
        }
        .data-table {
            width: 100%;
            border-collapse: collapse;
            margin: 10px 0;
        }
        .data-table th {
            background-color: #4CAF50;
            color: white;
            padding: 8px;
            text-align: left;
            border: 1px solid #ddd;
            font-weight: bold;
        }
        .data-table td {
            padding: 6px;
            border: 1px solid #ddd;
        }
        .data-table tr.even {
            background-color: #f9f9f9;
        }
        .data-table tr.odd {
            background-color: #ffffff;
        }
        .data-table tr.total {
            background-color: #e8f4e8;
            font-weight: bold;
        }
        .data-table tr:hover {
            background-color: #f5f5f5;
        }
        .footer {
            margin-top: 20px;
            text-align: right;
            font-size: 9pt;
            color: #666;
        }
    )";
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool ExportManager::saveToPdf(const QString &htmlContent, const QString &fileName,
                            QPageSize::PageSizeId pageSize)
{
    if (fileName.isEmpty()) return false;
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(pageSize));  // Для Qt6
    printer.setPageOrientation(QPageLayout::Landscape);
    
    QTextDocument document;
    document.setHtml(htmlContent);
    
    // Вычисляем оптимальный размер
    document.setPageSize(printer.pageRect(QPrinter::Point).size());
    
    // Печатаем
    document.print(&printer);
    
    return true;
}
#else
bool ExportManager::saveToPdf(const QString &htmlContent, const QString &fileName,
                            QPrinter::PageSize pageSize)
{
    if (fileName.isEmpty()) return false;
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(pageSize);  // Для Qt5
    printer.setPageOrientation(QPrinter::Landscape);
    
    QTextDocument document;
    document.setHtml(htmlContent);
    
    // Вычисляем оптимальный размер
    document.setPageSize(printer.pageRect(QPrinter::Point).size());
    
    // Печатаем
    document.print(&printer);
    
    return true;
}
#endif
