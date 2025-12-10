#ifndef ADDTRANSACTIONDIALOG_H
#define ADDTRANSACTIONDIALOG_H

#include <QDialog>

class QComboBox;
class QDateEdit;
class QLineEdit;
class QDoubleSpinBox;
class QPushButton;

class AddTransactionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTransactionDialog(QWidget *parent = nullptr);

protected slots:  // Меняем private на protected
    virtual void saveTransaction();  // Добавляем virtual
    void validateForm();

protected:  // Меняем private на protected для доступа в наследниках
    void setupUI();
    void loadAccounts();
    void loadCounterparties();

    // Элементы формы - делаем protected для доступа в наследниках
    QDateEdit *dateEdit;
    QComboBox *debitAccountCombo;
    QComboBox *creditAccountCombo;
    QDoubleSpinBox *amountSpin;
    QComboBox *counterpartyCombo;
    QLineEdit *descriptionEdit;
    QLineEdit *documentNumberEdit;
    QDateEdit *documentDateEdit;
    
    QPushButton *saveButton;
};

#endif // ADDTRANSACTIONDIALOG_H