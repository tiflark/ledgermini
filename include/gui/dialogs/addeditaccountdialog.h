#ifndef ADDEDITACCOUNTDIALOG_H
#define ADDEDITACCOUNTDIALOG_H

#include <QDialog>

class QLineEdit;
class QComboBox;
class QTextEdit;
class QPushButton;
class QLabel;

class AddEditAccountDialog : public QDialog
{
    Q_OBJECT

public:
    // Конструктор для добавления нового счета
    explicit AddEditAccountDialog(QWidget *parent = nullptr);
    // Конструктор для редактирования существующего счета
    AddEditAccountDialog(int accountId, QWidget *parent = nullptr);
    
    ~AddEditAccountDialog() = default;

private slots:
    void saveAccount();
    void validateForm();

private:
    void setupUI();
    void loadAccountData();
    void loadParentAccounts();
    
    int accountId_ = -1; // -1 для нового счета, >0 для редактирования
    bool isEditMode_ = false;
    
    // Элементы формы
    QLineEdit *codeEdit_;
    QLineEdit *nameEdit_;
    QComboBox *typeCombo_;
    QComboBox *parentCombo_;
    QTextEdit *descriptionEdit_;
    QPushButton *saveButton_;
    QLabel *codeValidationLabel_;
};

#endif // ADDEDITACCOUNTDIALOG_H