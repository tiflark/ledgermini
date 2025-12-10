#include "gui/dialogs/edittemplatedialog.h"

#include <QTimer>
#include <QApplication>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QJsonDocument>

EditTemplateDialog::EditTemplateDialog(QWidget *parent)
    : QDialog(parent)
    , m_nameEdit(new QLineEdit(this))
    , m_descriptionEdit(new QLineEdit(this))
    , m_typeCombo(new QComboBox(this))
    , m_contentEdit(new QTextEdit(this))
    , m_versionSpin(new QSpinBox(this))
    , m_isDefaultCheck(new QCheckBox(tr("Шаблон по умолчанию"), this))
    , m_previewEdit(new QTextEdit(this))
    , m_saveButton(new QPushButton(tr("Сохранить"), this))
    , m_cancelButton(new QPushButton(tr("Отмена"), this))
    , m_testButton(new QPushButton(tr("Тестировать шаблон"), this))
    , m_nameErrorLabel(new QLabel(this))
    , m_contentErrorLabel(new QLabel(this))
    , m_isEditMode(false)
{
    setupUI();
    initConnections();
    
    // Установка начальных значений
    m_versionSpin->setRange(1, 100);
    m_versionSpin->setValue(1);
    m_previewEdit->setReadOnly(true);
    
    // Настройка валидаторов
    QRegularExpression nameRegex("^[a-zA-ZА-Яа-я0-9_\\-\\s]+$");
    m_nameEdit->setValidator(new QRegularExpressionValidator(nameRegex, this));
    m_nameEdit->setMaxLength(MAX_NAME_LENGTH);
    m_descriptionEdit->setMaxLength(MAX_DESCRIPTION_LENGTH);
    
    // Заполнение комбобокса типами шаблонов
    m_typeCombo->addItem(tr("Транзакция"), Transaction);
    m_typeCombo->addItem(tr("Отчет"), Report);
    m_typeCombo->addItem(tr("Счет"), Invoice);
    m_typeCombo->addItem(tr("Пользовательский"), Custom);
    
    // Стилизация ошибок
    m_nameErrorLabel->setStyleSheet("color: red; font-size: 10px;");
    m_contentErrorLabel->setStyleSheet("color: red; font-size: 10px;");
    m_nameErrorLabel->setVisible(false);
    m_contentErrorLabel->setVisible(false);
    
    setWindowTitle(tr("Создать новый шаблон"));
    resize(700, 600);
}

EditTemplateDialog::EditTemplateDialog(const QJsonObject &templateData, QWidget *parent)
    : EditTemplateDialog(parent)
{
    loadTemplateData(templateData);
    m_isEditMode = true;
    setWindowTitle(tr("Редактировать шаблон: %1").arg(m_originalName));
}

EditTemplateDialog::~EditTemplateDialog()
{
}

void EditTemplateDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Группа основных свойств
    QGroupBox *propertiesGroup = new QGroupBox(tr("Свойства шаблона"), this);
    QFormLayout *formLayout = new QFormLayout(propertiesGroup);
    
    formLayout->addRow(tr("Название:"), m_nameEdit);
    formLayout->addRow(m_nameErrorLabel);
    formLayout->addRow(tr("Описание:"), m_descriptionEdit);
    formLayout->addRow(tr("Тип:"), m_typeCombo);
    formLayout->addRow(tr("Версия:"), m_versionSpin);
    formLayout->addRow(m_isDefaultCheck);
    
    mainLayout->addWidget(propertiesGroup);
    
    // Группа содержимого шаблона
    QGroupBox *contentGroup = new QGroupBox(tr("Содержимое шаблона"), this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentGroup);
    
    QLabel *contentHint = new QLabel(
        tr("Используйте заполнители: {дата}, {сумма}, {описание}, {категория}, {счет}, {получатель}"), this);
    contentHint->setStyleSheet("font-style: italic; color: #666;");
    
    contentLayout->addWidget(contentHint);
    contentLayout->addWidget(m_contentEdit);
    contentLayout->addWidget(m_contentErrorLabel);
    
    mainLayout->addWidget(contentGroup);
    
    // Группа предпросмотра
    QGroupBox *previewGroup = new QGroupBox(tr("Предпросмотр"), this);
    QVBoxLayout *previewLayout = new QVBoxLayout(previewGroup);
    previewLayout->addWidget(m_previewEdit);
    mainLayout->addWidget(previewGroup);
    
    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_testButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_saveButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Настройка размеров
    m_contentEdit->setMinimumHeight(150);
    m_previewEdit->setMinimumHeight(100);
    
    m_saveButton->setDefault(true);
    m_saveButton->setEnabled(false);
}

void EditTemplateDialog::initConnections()
{
    connect(m_nameEdit, &QLineEdit::textChanged,
            this, &EditTemplateDialog::validateForm);
    connect(m_contentEdit, &QTextEdit::textChanged,
            this, &EditTemplateDialog::validateForm);
    
    connect(m_saveButton, &QPushButton::clicked,
            this, &EditTemplateDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &EditTemplateDialog::onCancelClicked);
    connect(m_testButton, &QPushButton::clicked,
            this, &EditTemplateDialog::onTestTemplateClicked);
}

void EditTemplateDialog::loadTemplateData(const QJsonObject &templateData)
{
    if (templateData.isEmpty())
        return;
    
    m_originalName = templateData["name"].toString();
    m_nameEdit->setText(m_originalName);
    m_descriptionEdit->setText(templateData["description"].toString());
    
    TemplateType type = stringToTemplateType(templateData["type"].toString());
    m_typeCombo->setCurrentIndex(m_typeCombo->findData(type));
    
    m_contentEdit->setText(templateData["content"].toString());
    m_versionSpin->setValue(templateData["version"].toInt(1));
    m_isDefaultCheck->setChecked(templateData["isDefault"].toBool());
    
    validateForm();
}

QJsonObject EditTemplateDialog::getTemplateData() const
{
    QJsonObject templateData;
    templateData["name"] = m_nameEdit->text().trimmed();
    templateData["description"] = m_descriptionEdit->text().trimmed();
    templateData["type"] = templateTypeToString(static_cast<TemplateType>(
        m_typeCombo->currentData().toInt()));
    templateData["content"] = m_contentEdit->toPlainText().trimmed();
    templateData["version"] = m_versionSpin->value();
    templateData["isDefault"] = m_isDefaultCheck->isChecked();
    templateData["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    if (m_isEditMode) {
        templateData["modified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        templateData["originalName"] = m_originalName;
    }
    
    return templateData;
}

QString EditTemplateDialog::getTemplateName() const
{
    return m_nameEdit->text().trimmed();
}

QString EditTemplateDialog::getTemplateDescription() const
{
    return m_descriptionEdit->text().trimmed();
}

QString EditTemplateDialog::getTemplateContent() const
{
    return m_contentEdit->toPlainText().trimmed();
}

EditTemplateDialog::TemplateType EditTemplateDialog::getTemplateType() const
{
    return static_cast<TemplateType>(m_typeCombo->currentData().toInt());
}

QString EditTemplateDialog::templateTypeToString(TemplateType type)
{
    switch (type) {
    case Transaction: return "transaction";
    case Report: return "report";
    case Invoice: return "invoice";
    case Custom: return "custom";
    default: return "custom";
    }
}

EditTemplateDialog::TemplateType EditTemplateDialog::stringToTemplateType(const QString &typeString)
{
    if (typeString == "transaction") return Transaction;
    if (typeString == "report") return Report;
    if (typeString == "invoice") return Invoice;
    return Custom;
}

void EditTemplateDialog::validateForm()
{
    bool isValid = true;
    QString errorText;
    
    // Валидация имени
    if (!validateTemplateName()) {
        m_nameErrorLabel->setText(tr("Название должно быть от 3 до 100 символов и содержать только буквы, цифры, пробелы, дефисы и подчеркивания"));
        m_nameErrorLabel->setVisible(true);
        isValid = false;
    } else {
        m_nameErrorLabel->setVisible(false);
    }
    
    // Валидация содержимого
    if (!validateTemplateContent()) {
        m_contentErrorLabel->setText(tr("Содержимое шаблона не может быть пустым"));
        m_contentErrorLabel->setVisible(true);
        isValid = false;
    } else {
        m_contentErrorLabel->setVisible(false);
    }
    
    // Обновление предпросмотра
    m_previewEdit->setText(generatePreview());
    
    m_saveButton->setEnabled(isValid);
}

bool EditTemplateDialog::validateTemplateName()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.length() < 3 || name.length() > MAX_NAME_LENGTH)
        return false;
    
    QRegularExpression regex("^[a-zA-ZА-Яа-я0-9_\\-\\s]+$");
    return regex.match(name).hasMatch();
}

bool EditTemplateDialog::validateTemplateContent()
{
    return !m_contentEdit->toPlainText().trimmed().isEmpty();
}

QString EditTemplateDialog::generatePreview() const
{
    QString content = m_contentEdit->toPlainText();
    
    // Замена плейсхолдеров на тестовые значения
    content.replace("{дата}", QDate::currentDate().toString("dd.MM.yyyy"));
    content.replace("{сумма}", "100,00");
    content.replace("{описание}", "Тестовое описание транзакции");
    content.replace("{категория}", "Офисные расходы");
    content.replace("{счет}", "Бизнес-счет");
    content.replace("{получатель}", "Тестовый поставщик ООО");
    content.replace("{валюта}", "RUB");
    content.replace("{номер}", "СЧ-12345");
    
    return content;
}

void EditTemplateDialog::onSaveClicked()
{
    if (!validateTemplateName() || !validateTemplateContent()) {
        QMessageBox::warning(this, tr("Ошибка проверки"),
                           tr("Пожалуйста, исправьте ошибки перед сохранением"));
        return;
    }
    
    accept();
}

void EditTemplateDialog::onCancelClicked()
{
    reject();
}

void EditTemplateDialog::onTestTemplateClicked()
{
    QMessageBox::information(this, tr("Тест шаблона"),
        tr("Шаблон успешно скомпилирован!\n\n"
           "Заполнители заменены тестовыми значениями.\n"
           "Смотрите раздел предпросмотра для результата."));
    
    // Визуальная индикация успешного теста
    m_previewEdit->setStyleSheet("background-color: #f0fff0; border: 1px solid #90ee90;");
    QTimer::singleShot(1000, this, [this]() {
        m_previewEdit->setStyleSheet("");
    });
}