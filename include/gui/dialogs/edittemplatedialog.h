#ifndef EDITTEMPLATEDIALOG_H
#define EDITTEMPLATEDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include <QJsonArray>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QTextEdit;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QFormLayout;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
QT_END_NAMESPACE

class EditTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    enum TemplateType {
        Transaction,
        Report,
        Invoice,
        Custom
    };

    explicit EditTemplateDialog(QWidget *parent = nullptr);
    EditTemplateDialog(const QJsonObject &templateData, QWidget *parent = nullptr);
    ~EditTemplateDialog();

    QJsonObject getTemplateData() const;
    QString getTemplateName() const;
    QString getTemplateDescription() const;
    QString getTemplateContent() const;
    TemplateType getTemplateType() const;

    static QString templateTypeToString(TemplateType type);
    static TemplateType stringToTemplateType(const QString &typeString);

public slots:
    void validateForm();

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onTestTemplateClicked();

private:
    void setupUI();
    void initConnections();
    void loadTemplateData(const QJsonObject &templateData);
    
    // Валидация
    bool validateTemplateName();
    bool validateTemplateContent();
    
    // Генерация предпросмотра
    QString generatePreview() const;

    // UI элементы
    QLineEdit *m_nameEdit;
    QLineEdit *m_descriptionEdit;
    QComboBox *m_typeCombo;
    QTextEdit *m_contentEdit;
    QSpinBox *m_versionSpin;
    QCheckBox *m_isDefaultCheck;
    QTextEdit *m_previewEdit;
    
    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
    QPushButton *m_testButton;
    
    QLabel *m_nameErrorLabel;
    QLabel *m_contentErrorLabel;
    
    // Данные
    QString m_originalName;
    bool m_isEditMode;
    
    // Максимальные значения
    static const int MAX_NAME_LENGTH = 100;
    static const int MAX_DESCRIPTION_LENGTH = 500;
};

#endif // EDITTEMPLATEDIALOG_H