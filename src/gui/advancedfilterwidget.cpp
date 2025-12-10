#include "gui/advancedfilterwidget.h"

#include <QInputDialog>
#include <QApplication>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

AdvancedFilterWidget::AdvancedFilterWidget(QWidget *parent)
    : QWidget(parent)
    , searchEdit(new QLineEdit(this))
    , fieldCombo(new QComboBox(this))
    , dateGroup(new QGroupBox(tr("Фильтр по дате"), this))
    , dateFromEdit(new QDateEdit(this))
    , dateToEdit(new QDateEdit(this))
    , amountGroup(new QGroupBox(tr("Фильтр по сумме"), this))
    , amountFromSpin(new QDoubleSpinBox(this))
    , amountToSpin(new QDoubleSpinBox(this))
    , debitAccountCombo(new QComboBox(this))
    , creditAccountCombo(new QComboBox(this))
    , counterpartyCombo(new QComboBox(this))
    , savedFilterGroup(new QGroupBox(tr("Сохраненные фильтры"), this))
    , savedFilterCombo(new QComboBox(this))
    , saveFilterButton(new QPushButton(tr("Сохранить"), this))
    , loadFilterButton(new QPushButton(tr("Загрузить"), this))
    , deleteFilterButton(new QPushButton(tr("Удалить"), this))
    , applyButton(new QPushButton(tr("Применить фильтр"), this))
    , clearButton(new QPushButton(tr("Сбросить"), this))
{
    setupUI();
    initConnections();
    
    // Установка начальных значений
    QDate today = QDate::currentDate();
    dateFromEdit->setDate(today.addMonths(-1));
    dateToEdit->setDate(today);
    dateFromEdit->setCalendarPopup(true);
    dateToEdit->setCalendarPopup(true);
    
    amountFromSpin->setRange(-9999999.99, 9999999.99);
    amountToSpin->setRange(-9999999.99, 9999999.99);
    amountFromSpin->setDecimals(2);
    amountToSpin->setDecimals(2);
    amountFromSpin->setValue(0.0);
    amountToSpin->setValue(10000.0);
    
    // Заполнение комбобоксов
    fieldCombo->addItem(tr("Любое поле"), "");
    fieldCombo->addItem(tr("Описание"), "description");
    fieldCombo->addItem(tr("Комментарий"), "comment");
    fieldCombo->addItem(tr("Категория"), "category");
    fieldCombo->addItem(tr("Тег"), "tag");
    
    debitAccountCombo->addItem(tr("Любой счет"), -1);
    creditAccountCombo->addItem(tr("Любой счет"), -1);
    counterpartyCombo->addItem(tr("Любой контрагент"), -1);
    
    // Загрузка сохраненных фильтров
    loadSavedFiltersList();
    
    // Настройка валидации дат
    dateToEdit->setMinimumDate(dateFromEdit->date());
    
    // Установка стилей
    applyButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; }");
    clearButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; }");
    saveFilterButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; }");
}

void AdvancedFilterWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Поиск по тексту
    QGroupBox *textGroup = new QGroupBox(tr("Текстовый поиск"), this);
    QFormLayout *textLayout = new QFormLayout(textGroup);
    textLayout->addRow(tr("Текст:"), searchEdit);
    textLayout->addRow(tr("Поле:"), fieldCombo);
    mainLayout->addWidget(textGroup);
    
    // Фильтр по дате
    QFormLayout *dateLayout = new QFormLayout(dateGroup);
    dateLayout->addRow(tr("С:"), dateFromEdit);
    dateLayout->addRow(tr("По:"), dateToEdit);
    mainLayout->addWidget(dateGroup);
    
    // Фильтр по сумме
    QFormLayout *amountLayout = new QFormLayout(amountGroup);
    amountLayout->addRow(tr("От:"), amountFromSpin);
    amountLayout->addRow(tr("До:"), amountToSpin);
    mainLayout->addWidget(amountGroup);
    
    // Фильтры по счетам и контрагентам
    QGroupBox *accountGroup = new QGroupBox(tr("Фильтр по счетам и контрагентам"), this);
    QFormLayout *accountLayout = new QFormLayout(accountGroup);
    accountLayout->addRow(tr("Счет дебета:"), debitAccountCombo);
    accountLayout->addRow(tr("Счет кредита:"), creditAccountCombo);
    accountLayout->addRow(tr("Контрагент:"), counterpartyCombo);
    mainLayout->addWidget(accountGroup);
    
    // Сохраненные фильтры
    QVBoxLayout *savedLayout = new QVBoxLayout(savedFilterGroup);
    savedLayout->addWidget(savedFilterCombo);
    
    QHBoxLayout *filterButtonsLayout = new QHBoxLayout();
    filterButtonsLayout->addWidget(saveFilterButton);
    filterButtonsLayout->addWidget(loadFilterButton);
    filterButtonsLayout->addWidget(deleteFilterButton);
    savedLayout->addLayout(filterButtonsLayout);
    mainLayout->addWidget(savedFilterGroup);
    
    // Кнопки управления фильтром
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    mainLayout->addStretch();
    
    // Настройка размеров
    savedFilterCombo->setEditable(true);
    searchEdit->setPlaceholderText(tr("Введите текст для поиска..."));
    savedFilterCombo->lineEdit()->setPlaceholderText(tr("Имя фильтра"));
}

void AdvancedFilterWidget::initConnections()
{
    connect(dateFromEdit, &QDateEdit::dateChanged, this, [this](const QDate &date) {
        dateToEdit->setMinimumDate(date);
    });
    
    connect(applyButton, &QPushButton::clicked, this, &AdvancedFilterWidget::onApplyClicked);
    connect(clearButton, &QPushButton::clicked, this, &AdvancedFilterWidget::onClearClicked);
    connect(saveFilterButton, &QPushButton::clicked, this, &AdvancedFilterWidget::onSaveClicked);
    connect(loadFilterButton, &QPushButton::clicked, this, &AdvancedFilterWidget::onLoadClicked);
    connect(deleteFilterButton, &QPushButton::clicked, this, [this]() {
        QString filterName = savedFilterCombo->currentText();
        if (!filterName.isEmpty() && savedFilters.contains(filterName)) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, tr("Удаление фильтра"),
                tr("Вы уверены, что хотите удалить фильтр '%1'?").arg(filterName),
                QMessageBox::Yes | QMessageBox::No
            );
            
            if (reply == QMessageBox::Yes) {
                savedFilters.remove(filterName);
                loadSavedFiltersList();
                QMessageBox::information(this, tr("Фильтр удален"),
                    tr("Фильтр '%1' был успешно удален.").arg(filterName));
            }
        }
    });
}

void AdvancedFilterWidget::loadAccounts()
{
    // В реальном приложении здесь должна быть загрузка счетов из базы данных
    // Для примера добавляем тестовые счета
    debitAccountCombo->clear();
    creditAccountCombo->clear();
    
    debitAccountCombo->addItem(tr("Любой счет"), -1);
    creditAccountCombo->addItem(tr("Любой счет"), -1);
    
    debitAccountCombo->addItem(tr("Наличные"), 1);
    creditAccountCombo->addItem(tr("Наличные"), 1);
    
    debitAccountCombo->addItem(tr("Банковский счет"), 2);
    creditAccountCombo->addItem(tr("Банковский счет"), 2);
    
    debitAccountCombo->addItem(tr("Кредитная карта"), 3);
    creditAccountCombo->addItem(tr("Кредитная карта"), 3);
    
    debitAccountCombo->addItem(tr("Электронный кошелек"), 4);
    creditAccountCombo->addItem(tr("Электронный кошелек"), 4);
}

void AdvancedFilterWidget::loadCounterparties()
{
    // В реальном приложении здесь должна быть загрузка контрагентов из базы данных
    // Для примера добавляем тестовых контрагентов
    counterpartyCombo->clear();
    counterpartyCombo->addItem(tr("Любой контрагент"), -1);
    counterpartyCombo->addItem(tr("Магазин продуктов"), 1);
    counterpartyCombo->addItem(tr("Аптека"), 2);
    counterpartyCombo->addItem(tr("Транспорт"), 3);
    counterpartyCombo->addItem(tr("Ресторан"), 4);
    counterpartyCombo->addItem(tr("Интернет-магазин"), 5);
}

void AdvancedFilterWidget::loadSavedFiltersList()
{
    savedFilterCombo->clear();
    savedFilterCombo->addItem(tr("-- Выберите фильтр --"), "");
    
    // Загрузка из QSettings (в реальном приложении - из базы данных)
    QSettings settings;
    settings.beginGroup("SavedFilters");
    
    QStringList filterNames = settings.childGroups();
    for (const QString &name : filterNames) {
        savedFilterCombo->addItem(name, name);
        
        // Загружаем сам фильтр
        FilterOptions options;
        options.textFilter = settings.value(name + "/textFilter").toString();
        options.fieldFilter = settings.value(name + "/fieldFilter").toString();
        options.dateFrom = settings.value(name + "/dateFrom").toDate();
        options.dateTo = settings.value(name + "/dateTo").toDate();
        options.amountFrom = settings.value(name + "/amountFrom").toDouble();
        options.amountTo = settings.value(name + "/amountTo").toDouble();
        options.debitAccountId = settings.value(name + "/debitAccountId").toInt();
        options.creditAccountId = settings.value(name + "/creditAccountId").toInt();
        options.counterpartyId = settings.value(name + "/counterpartyId").toInt();
        
        savedFilters[name] = options;
    }
    
    settings.endGroup();
    
    if (savedFilters.isEmpty()) {
        savedFilterCombo->setEnabled(false);
        loadFilterButton->setEnabled(false);
        deleteFilterButton->setEnabled(false);
    } else {
        savedFilterCombo->setEnabled(true);
        loadFilterButton->setEnabled(true);
        deleteFilterButton->setEnabled(true);
    }
}

AdvancedFilterWidget::FilterOptions AdvancedFilterWidget::getFilterOptions() const
{
    FilterOptions options;
    options.textFilter = searchEdit->text().trimmed();
    options.fieldFilter = fieldCombo->currentData().toString();
    options.dateFrom = dateFromEdit->date();
    options.dateTo = dateToEdit->date();
    options.dateFilterEnabled = dateGroup->isChecked();
    options.amountFrom = amountFromSpin->value();
    options.amountTo = amountToSpin->value();
    options.amountFilterEnabled = amountGroup->isChecked();
    options.debitAccountId = debitAccountCombo->currentData().toInt();
    options.creditAccountId = creditAccountCombo->currentData().toInt();
    options.counterpartyId = counterpartyCombo->currentData().toInt();
    options.useSavedFilter = !savedFilterCombo->currentData().toString().isEmpty();
    options.savedFilterName = savedFilterCombo->currentText();
    
    return options;
}

void AdvancedFilterWidget::setFilterOptions(const FilterOptions &options)
{
    searchEdit->setText(options.textFilter);
    
    int fieldIndex = fieldCombo->findData(options.fieldFilter);
    if (fieldIndex >= 0) {
        fieldCombo->setCurrentIndex(fieldIndex);
    }
    
    dateFromEdit->setDate(options.dateFrom);
    dateToEdit->setDate(options.dateTo);
    dateGroup->setChecked(options.dateFilterEnabled);
    
    amountFromSpin->setValue(options.amountFrom);
    amountToSpin->setValue(options.amountTo);
    amountGroup->setChecked(options.amountFilterEnabled);
    
    int debitIndex = debitAccountCombo->findData(options.debitAccountId);
    if (debitIndex >= 0) {
        debitAccountCombo->setCurrentIndex(debitIndex);
    }
    
    int creditIndex = creditAccountCombo->findData(options.creditAccountId);
    if (creditIndex >= 0) {
        creditAccountCombo->setCurrentIndex(creditIndex);
    }
    
    int counterpartyIndex = counterpartyCombo->findData(options.counterpartyId);
    if (counterpartyIndex >= 0) {
        counterpartyCombo->setCurrentIndex(counterpartyIndex);
    }
    
    if (!options.savedFilterName.isEmpty()) {
        int savedIndex = savedFilterCombo->findText(options.savedFilterName);
        if (savedIndex >= 0) {
            savedFilterCombo->setCurrentIndex(savedIndex);
        }
    }
}

void AdvancedFilterWidget::clearFilters()
{
    searchEdit->clear();
    fieldCombo->setCurrentIndex(0);
    
    QDate today = QDate::currentDate();
    dateFromEdit->setDate(today.addMonths(-1));
    dateToEdit->setDate(today);
    dateGroup->setChecked(false);
    
    amountFromSpin->setValue(0.0);
    amountToSpin->setValue(10000.0);
    amountGroup->setChecked(false);
    
    debitAccountCombo->setCurrentIndex(0);
    creditAccountCombo->setCurrentIndex(0);
    counterpartyCombo->setCurrentIndex(0);
    
    savedFilterCombo->setCurrentIndex(0);
}

void AdvancedFilterWidget::saveFilter(const QString &name)
{
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"), 
            tr("Пожалуйста, введите имя для сохранения фильтра."));
        return;
    }
    
    FilterOptions options = getFilterOptions();
    options.savedFilterName = name;
    
    // Сохранение в QSettings
    QSettings settings;
    settings.beginGroup("SavedFilters/" + name);
    
    settings.setValue("textFilter", options.textFilter);
    settings.setValue("fieldFilter", options.fieldFilter);
    settings.setValue("dateFrom", options.dateFrom);
    settings.setValue("dateTo", options.dateTo);
    settings.setValue("amountFrom", options.amountFrom);
    settings.setValue("amountTo", options.amountTo);
    settings.setValue("debitAccountId", options.debitAccountId);
    settings.setValue("creditAccountId", options.creditAccountId);
    settings.setValue("counterpartyId", options.counterpartyId);
    
    settings.endGroup();
    settings.sync();
    
    // Добавляем в список, если его там нет
    if (!savedFilters.contains(name)) {
        savedFilters[name] = options;
        savedFilterCombo->addItem(name, name);
    }
    
    savedFilterCombo->setCurrentText(name);
    loadSavedFiltersList();
    
    QMessageBox::information(this, tr("Фильтр сохранен"),
        tr("Фильтр '%1' успешно сохранен.").arg(name));
    
    emit filterSaved(name);
}

void AdvancedFilterWidget::loadFilter(const QString &name)
{
    if (!savedFilters.contains(name)) {
        QMessageBox::warning(this, tr("Ошибка"),
            tr("Фильтр '%1' не найден.").arg(name));
        return;
    }
    
    setFilterOptions(savedFilters[name]);
    savedFilterCombo->setCurrentText(name);
    
    emit filterLoaded(name);
}

QStringList AdvancedFilterWidget::getSavedFilters() const
{
    return savedFilters.keys();
}

void AdvancedFilterWidget::onApplyClicked()
{
    // Валидация фильтров
    if (dateGroup->isChecked() && dateFromEdit->date() > dateToEdit->date()) {
        QMessageBox::warning(this, tr("Ошибка даты"),
            tr("Дата 'С' не может быть позже даты 'По'."));
        return;
    }
    
    if (amountGroup->isChecked() && amountFromSpin->value() > amountToSpin->value()) {
        QMessageBox::warning(this, tr("Ошибка суммы"),
            tr("Сумма 'От' не может быть больше суммы 'До'."));
        return;
    }
    
    emit filterApplied();
}

void AdvancedFilterWidget::onClearClicked()
{
    clearFilters();
    emit filterApplied(); // Сбрасываем фильтр, показывая все записи
}

void AdvancedFilterWidget::onSaveClicked()
{
    QString filterName = savedFilterCombo->currentText();
    if (filterName.isEmpty() || filterName == tr("-- Выберите фильтр --")) {
        filterName = savedFilterCombo->lineEdit()->placeholderText();
        if (filterName.isEmpty()) {
            filterName = tr("Новый фильтр");
        }
    }
    
    bool ok;
    QString name = QInputDialog::getText(this, tr("Сохранение фильтра"),
        tr("Введите имя фильтра:"), QLineEdit::Normal, filterName, &ok);
    
    if (ok && !name.isEmpty()) {
        saveFilter(name);
    }
}

void AdvancedFilterWidget::onLoadClicked()
{
    QString filterName = savedFilterCombo->currentText();
    if (filterName.isEmpty() || filterName == tr("-- Выберите фильтр --")) {
        QMessageBox::warning(this, tr("Ошибка"),
            tr("Пожалуйста, выберите фильтр для загрузки."));
        return;
    }
    
    loadFilter(filterName);
}