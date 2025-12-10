#include "gui/operationsjournalwidget.h"
#include "gui/advancedfilterwidget.h"

#include <QLabel>              // <-- ДОБАВЬТЕ
#include <QRandomGenerator>    // <-- ДОБАВЬТЕ
#include <QStringConverter> 
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QPushButton>
#include <QToolBar>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSortFilterProxyModel>
#include <QDate>
#include <QDebug>
#include <QFileDialog>
#include <QTextDocument>
#include <QPrinter>
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextLength>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QDir>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

OperationsJournalWidget::OperationsJournalWidget(QWidget *parent)
    : QWidget(parent)
    , filterWidget(new AdvancedFilterWidget(this))
    , tableView(new QTableView(this))
    , model(new QStandardItemModel(this))
    , refreshButton(new QPushButton(tr("Обновить"), this))
    , pdfButton(new QPushButton(tr("Экспорт в PDF"), this))
    , excelButton(new QPushButton(tr("Экспорт в Excel"), this))
{
    setupUI();
    initConnections();
    loadData();
}

void OperationsJournalWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(2, 2, 2, 2);  // Минимальные отступы
    
    // Панель инструментов
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    toolbarLayout->addWidget(filterWidget);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(pdfButton);
    buttonLayout->addWidget(excelButton);
    buttonLayout->addStretch();
    
    toolbarLayout->addLayout(buttonLayout);
    mainLayout->addLayout(toolbarLayout);
    
    // Таблица
    tableView->setModel(model);
    tableView->setAlternatingRowColors(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSortingEnabled(true);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // Настройка заголовков
    QStringList headers;
    headers << tr("ID") 
            << tr("Дата") 
            << tr("Счет дебета") 
            << tr("Счет кредита") 
            << tr("Сумма") 
            << tr("Валюта") 
            << tr("Описание") 
            << tr("Категория") 
            << tr("Контрагент") 
            << tr("Теги") 
            << tr("Создано");
    
    model->setHorizontalHeaderLabels(headers);
    
    // Настройка ширины столбцов - сделаем адаптивными
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->setDefaultSectionSize(25);
    
    // Автоматически подгоняем ширину столбцов под содержимое
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableView->setColumnWidth(0, 50);   // ID
    tableView->setColumnWidth(1, 80);   // Дата
    tableView->setColumnWidth(2, 120);  // Счет дебета
    tableView->setColumnWidth(3, 120);  // Счет кредита
    tableView->setColumnWidth(4, 80);   // Сумма
    tableView->setColumnWidth(5, 70);   // Валюта
    tableView->setColumnWidth(6, 200);  // Описание
    tableView->setColumnWidth(7, 120);  // Категория
    tableView->setColumnWidth(8, 120);  // Контрагент
    tableView->setColumnWidth(9, 150);  // Теги
    tableView->setColumnWidth(10, 120); // Создано
    
    // Включаем растягивание столбцов при изменении размера окна
    tableView->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch); // Описание
    
    // Подсветка отрицательных сумм
    tableView->setStyleSheet(
        "QTableView { background-color: white; }"
        "QTableView::item { padding: 2px; }"
        "QTableView::item:selected { background-color: #4CAF50; color: white; }"
        "QTableView QHeaderView::section { background-color: #f0f0f0; padding: 4px; }"
    );
    
    // Таблица должна растягиваться на все доступное пространство
    mainLayout->addWidget(tableView, 1);  // Второй параметр 1 - stretch factor
    
    // Статус бар
    statusLabel = new QLabel(tr("Загружено записей: 0"), this);
    statusLabel->setMinimumHeight(20);
    mainLayout->addWidget(statusLabel);
    
    // Настройка кнопок
    refreshButton->setIcon(QIcon::fromTheme("view-refresh"));
    pdfButton->setIcon(QIcon::fromTheme("document-export"));
    excelButton->setIcon(QIcon::fromTheme("x-office-spreadsheet"));
    
    refreshButton->setToolTip(tr("Обновить данные журнала"));
    pdfButton->setToolTip(tr("Экспортировать в PDF файл"));
    excelButton->setToolTip(tr("Экспортировать в Excel (CSV) файл"));
}

void OperationsJournalWidget::initConnections()
{
    connect(refreshButton, &QPushButton::clicked, this, &OperationsJournalWidget::refreshJournal);
    connect(pdfButton, &QPushButton::clicked, this, &OperationsJournalWidget::exportToPdf);
    connect(excelButton, &QPushButton::clicked, this, &OperationsJournalWidget::exportToExcel);
    connect(filterWidget, &AdvancedFilterWidget::filterApplied, this, &OperationsJournalWidget::onFilterApplied);
    
    // Двойной клик по записи
    connect(tableView, &QTableView::doubleClicked, this, [this](const QModelIndex &index) {
        if (index.isValid()) {
            int id = model->data(model->index(index.row(), 0)).toInt();
            qDebug() << tr("Открыть запись ID:") << id;
            // В реальном приложении здесь открытие диалога редактирования
            QMessageBox::information(this, tr("Детали записи"),
                tr("Запись ID: %1\nДвойной клик для редактирования").arg(id));
        }
    });
}

void OperationsJournalWidget::loadData()
{
    model->removeRows(0, model->rowCount());
    
    // В реальном приложении здесь запрос к базе данных
    // Для примера создадим тестовые данные
    
    for (int i = 1; i <= 50; ++i) {
        QList<QStandardItem*> rowItems;
        
        // ID
        rowItems.append(new QStandardItem(QString::number(i)));
        
        // Дата (рандомная за последние 30 дней)
        int daysAgo = QRandomGenerator::global()->bounded(30);
        QDate date = QDate::currentDate().addDays(-daysAgo);
        rowItems.append(new QStandardItem(date.toString("dd.MM.yyyy")));
        
        // Счет дебета
        QStringList debitAccounts = {tr("Наличные"), tr("Банковский счет"), tr("Кредитная карта")};
        rowItems.append(new QStandardItem(debitAccounts[QRandomGenerator::global()->bounded(3)]));
        
        // Счет кредита
        QStringList creditAccounts = {tr("Магазин"), tr("Ресторан"), tr("Транспорт"), tr("Развлечения")};
        rowItems.append(new QStandardItem(creditAccounts[QRandomGenerator::global()->bounded(4)]));
        
        // Сумма (от -5000 до 5000)
        double amount = (QRandomGenerator::global()->bounded(10000) - 5000) / 100.0;
        QStandardItem *amountItem = new QStandardItem(QString::number(amount, 'f', 2));
        if (amount < 0) {
            amountItem->setForeground(QBrush(Qt::red));
        } else if (amount > 0) {
            amountItem->setForeground(QBrush(Qt::darkGreen));
        }
        rowItems.append(amountItem);
        
        // Валюта
        rowItems.append(new QStandardItem("RUB"));
        
        // Описание
        QStringList descriptions = {
            tr("Покупка продуктов"),
            tr("Оплата услуг"),
            tr("Перевод между счетами"),
            tr("Зарплата"),
            tr("Оплата аренды"),
            tr("Пополнение счета")
        };
        rowItems.append(new QStandardItem(descriptions[QRandomGenerator::global()->bounded(6)]));
        
        // Категория
        QStringList categories = {
            tr("Продукты"), tr("Транспорт"), tr("Развлечения"),
            tr("Коммунальные услуги"), tr("Здоровье"), tr("Образование")
        };
        rowItems.append(new QStandardItem(categories[QRandomGenerator::global()->bounded(6)]));
        
        // Контрагент
        QStringList counterparties = {
            tr("Магазин Ашан"), tr("Ресторан Пушкин"), tr("Такси Яндекс"),
            tr("Кинотеатр"), tr("Больница №1"), tr("Университет")
        };
        rowItems.append(new QStandardItem(counterparties[QRandomGenerator::global()->bounded(6)]));
        
        // Теги
        QStringList tags = {"#покупка", "#еда", "#транспорт", "#развлечение", "#здоровье"};
        QString tagString;
        int tagCount = QRandomGenerator::global()->bounded(3) + 1;
        for (int t = 0; t < tagCount; ++t) {
            if (!tagString.isEmpty()) tagString += ", ";
            tagString += tags[QRandomGenerator::global()->bounded(tags.size())];
        }
        rowItems.append(new QStandardItem(tagString));
        
        // Дата создания
        QDateTime created = QDateTime::currentDateTime().addSecs(-i * 3600);
        rowItems.append(new QStandardItem(created.toString("dd.MM.yyyy HH:mm")));
        
        model->appendRow(rowItems);
    }
    
    // Обновляем статус - создаем лейбл, если его нет
    if (statusLabel) {
        statusLabel->setText(tr("Загружено записей: %1").arg(model->rowCount()));
    }
}

void OperationsJournalWidget::refreshJournal()
{
    loadData();
    QMessageBox::information(this, tr("Обновление"), 
        tr("Журнал операций успешно обновлен.\nЗагружено %1 записей.").arg(model->rowCount()));
}

void OperationsJournalWidget::applyFilter(const AdvancedFilterWidget::FilterOptions &options)
{
    // Очищаем текущие данные
    model->removeRows(0, model->rowCount());
    
    // В реальном приложении здесь SQL-запрос с фильтрами
    // Для примера просто загружаем тестовые данные
    loadData();
    
    // Применяем фильтры к уже загруженным данным (в реальном приложении это делается на уровне SQL)
    QString query = buildQuery(options);
    qDebug() << tr("Сгенерированный запрос:") << query;
    
    // Фильтрация по тексту (если задан)
    if (!options.textFilter.isEmpty()) {
        QList<QStandardItem*> foundItems;
        int rows = model->rowCount();
        int cols = model->columnCount();
        
        for (int row = 0; row < rows; ++row) {
            bool found = false;
            for (int col = 0; col < cols; ++col) {
                QStandardItem *item = model->item(row, col);
                if (item && item->text().contains(options.textFilter, Qt::CaseInsensitive)) {
                    found = true;
                    break;
                }
            }
            if (found) {
                // Сохраняем индексы строк, которые нужно оставить
                for (int col = 0; col < cols; ++col) {
                    foundItems.append(model->takeItem(row, col));
                }
            }
        }
        
        // Очищаем модель и добавляем отфильтрованные строки
        model->removeRows(0, model->rowCount());
        for (int i = 0; i < foundItems.size(); i += cols) {
            QList<QStandardItem*> rowItems;
            for (int j = 0; j < cols && (i + j) < foundItems.size(); ++j) {
                rowItems.append(foundItems.at(i + j));
            }
            if (!rowItems.isEmpty()) {
                model->appendRow(rowItems);
            }
        }
    }
    
    // Обновляем статус
    if (statusLabel) {
        statusLabel->setText(tr("Отфильтровано записей: %1").arg(model->rowCount()));
    }
}

QString OperationsJournalWidget::buildQuery(const AdvancedFilterWidget::FilterOptions &options)
{
    QString query = "SELECT * FROM operations WHERE 1=1";
    
    // Текстовый фильтр
    if (!options.textFilter.isEmpty()) {
        QString field = options.fieldFilter.isEmpty() ? 
            "description, comment, category, tag" : options.fieldFilter;
        query += QString(" AND (%1 LIKE '%%2%')").arg(field).arg(options.textFilter);
    }
    
    // Фильтр по дате
    if (options.dateFilterEnabled) {
        query += QString(" AND operation_date BETWEEN '%1' AND '%2'")
            .arg(options.dateFrom.toString("yyyy-MM-dd"))
            .arg(options.dateTo.toString("yyyy-MM-dd"));
    }
    
    // Фильтр по сумме
    if (options.amountFilterEnabled) {
        query += QString(" AND amount BETWEEN %1 AND %2")
            .arg(options.amountFrom)
            .arg(options.amountTo);
    }
    
    // Фильтр по счетам
    if (options.debitAccountId > 0) {
        query += QString(" AND debit_account_id = %1").arg(options.debitAccountId);
    }
    
    if (options.creditAccountId > 0) {
        query += QString(" AND credit_account_id = %1").arg(options.creditAccountId);
    }
    
    // Фильтр по контрагенту
    if (options.counterpartyId > 0) {
        query += QString(" AND counterparty_id = %1").arg(options.counterpartyId);
    }
    
    query += " ORDER BY operation_date DESC, id DESC";
    
    return query;
}

void OperationsJournalWidget::onFilterApplied()
{
    AdvancedFilterWidget::FilterOptions options = filterWidget->getFilterOptions();
    applyFilter(options);
}

void OperationsJournalWidget::exportToPdf()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Экспорт в PDF"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + 
            "/operations_journal_" + QDate::currentDate().toString("yyyy_MM_dd") + ".pdf",
        tr("PDF файлы (*.pdf)"));
    
    if (fileName.isEmpty()) {
        return;
    }
    
    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) {
        fileName += ".pdf";
    }
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setCreator("LedgerMini");
    printer.setDocName(tr("Журнал операций"));
    
    QTextDocument document;
    
    // Форматирование документа
    QTextCursor cursor(&document);
    
    // Заголовок
    QTextCharFormat titleFormat;
    titleFormat.setFontPointSize(14);
    titleFormat.setFontWeight(QFont::Bold);
    titleFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    cursor.setCharFormat(titleFormat);
    cursor.insertText(tr("ЖУРНАЛ ОПЕРАЦИЙ\n"));
    
    // Подзаголовок
    QTextCharFormat subtitleFormat;
    subtitleFormat.setFontPointSize(10);
    subtitleFormat.setFontItalic(true);
    cursor.setCharFormat(subtitleFormat);
    cursor.insertText(tr("Сгенерировано: %1\n\n").arg(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm")));
    
    // Создание таблицы
    QTextTableFormat tableFormat;
    tableFormat.setAlignment(Qt::AlignHCenter);
    tableFormat.setCellSpacing(0);
    tableFormat.setCellPadding(2);
    tableFormat.setBorder(1);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    
    QTextTable *table = cursor.insertTable(model->rowCount() + 1, model->columnCount(), tableFormat);
    
    // Заголовки столбцов
    QTextCharFormat headerFormat;
    headerFormat.setFontWeight(QFont::Bold);
    headerFormat.setBackground(QColor(240, 240, 240));
    
    for (int col = 0; col < model->columnCount(); ++col) {
        QTextTableCell cell = table->cellAt(0, col);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.setCharFormat(headerFormat);
        cellCursor.insertText(model->horizontalHeaderItem(col)->text());
    }
    
    // Данные
    QTextCharFormat dataFormat;
    dataFormat.setFontPointSize(8);
    
    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            QStandardItem *item = model->item(row, col);
            if (item) {
                QTextTableCell cell = table->cellAt(row + 1, col);
                QTextCursor cellCursor = cell.firstCursorPosition();
                
                // Особое форматирование для суммы
                if (col == 4) { // Столбец суммы
                    QTextCharFormat amountFormat = dataFormat;
                    QString text = item->text();
                    bool ok;
                    double amount = text.toDouble(&ok);
                    if (ok) {
                        if (amount < 0) {
                            amountFormat.setForeground(QBrush(Qt::red));
                        } else if (amount > 0) {
                            amountFormat.setForeground(QBrush(Qt::darkGreen));
                        }
                    }
                    cellCursor.setCharFormat(amountFormat);
                } else {
                    cellCursor.setCharFormat(dataFormat);
                }
                
                cellCursor.insertText(item->text());
            }
        }
    }
    
    // Подвал
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertText(tr("\nВсего записей: %1").arg(model->rowCount()));
    
    // Печать
    document.print(&printer);
    
    QMessageBox::information(this, tr("Экспорт завершен"),
        tr("Данные успешно экспортированы в PDF файл:\n%1").arg(fileName));
}

void OperationsJournalWidget::exportToExcel()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Экспорт в Excel (CSV)"),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + 
            "/operations_journal_" + QDate::currentDate().toString("yyyy_MM_dd") + ".csv",
        tr("CSV файлы (*.csv);;Все файлы (*)"));
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Ошибка"),
            tr("Не удалось открыть файл для записи:\n%1").arg(file.errorString()));
        return;
    }
    
    QTextStream out(&file);
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        out.setEncoding(QStringConverter::Utf8);
    #else
        out.setCodec("UTF-8");
    #endif
    
    // Заголовки
    for (int col = 0; col < model->columnCount(); ++col) {
        if (col > 0) out << ";";
        QString header = model->horizontalHeaderItem(col)->text();
        // Экранирование для CSV
        if (header.contains(';') || header.contains('"')) {
            header = '"' + header.replace('"', "\"\"") + '"';
        }
        out << header;
    }
    out << "\n";
    
    // Данные
    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            if (col > 0) out << ";";
            QStandardItem *item = model->item(row, col);
            if (item) {
                QString text = item->text();
                // Экранирование для CSV
                if (text.contains(';') || text.contains('"')) {
                    text = '"' + text.replace('"', "\"\"") + '"';
                }
                out << text;
            }
        }
        out << "\n";
    }
    
    file.close();
    
    QMessageBox::information(this, tr("Экспорт завершен"),
        tr("Данные успешно экспортированы в CSV файл:\n%1\n\n"
           "Формат: CSV с разделителем ';' и кодировкой UTF-8").arg(fileName));
}