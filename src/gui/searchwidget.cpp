#include "searchwidget.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QIcon>

SearchWidget::SearchWidget(QWidget *parent)
    : QWidget(parent)
    , searchEdit(new QLineEdit(this))
    , fieldCombo(new QComboBox(this))
    , searchButton(new QPushButton(tr("Поиск"), this))
    , clearButton(new QPushButton(tr("Очистить"), this))
{
    setupUI();
    
    // Настройка начальных значений
    searchEdit->setPlaceholderText(tr("Введите текст для поиска..."));
    
    // Добавляем поля для поиска
    addSearchField("all", tr("Во всех полях"));
    addSearchField("description", tr("Описание"));
    addSearchField("category", tr("Категория"));
    addSearchField("account", tr("Счет"));
    addSearchField("counterparty", tr("Контрагент"));
    
    // Подключение сигналов
    connect(searchButton, &QPushButton::clicked, this, &SearchWidget::onSearchClicked);
    connect(clearButton, &QPushButton::clicked, this, &SearchWidget::clear);
    connect(searchEdit, &QLineEdit::returnPressed, this, &SearchWidget::onSearchClicked);
    connect(searchEdit, &QLineEdit::textChanged, this, &SearchWidget::onTextChanged);
}

void SearchWidget::setupUI()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    layout->addWidget(fieldCombo);
    layout->addWidget(searchEdit);
    layout->addWidget(searchButton);
    layout->addWidget(clearButton);
    
    // Настройка размеров
    fieldCombo->setMaximumWidth(150);
    searchButton->setMaximumWidth(80);
    clearButton->setMaximumWidth(80);
    
    // Настройка кнопок
    searchButton->setEnabled(false);
}

QString SearchWidget::getSearchText() const
{
    return searchEdit->text().trimmed();
}

QString SearchWidget::getSearchField() const
{
    return fieldCombo->currentData().toString();
}

void SearchWidget::setPlaceholderText(const QString &text)
{
    searchEdit->setPlaceholderText(text);
}

void SearchWidget::addSearchField(const QString &field, const QString &displayName)
{
    fieldCombo->addItem(displayName, field);
}

void SearchWidget::clear()
{
    searchEdit->clear();
    emit searchRequested("", "");
}

void SearchWidget::onSearchClicked()
{
    QString text = getSearchText();
    QString field = getSearchField();
    emit searchRequested(text, field);
}

void SearchWidget::onTextChanged(const QString &text)
{
    searchButton->setEnabled(!text.trimmed().isEmpty());
}
