#ifndef EDITCOUNTERPARTYDIALOG_H
#define EDITCOUNTERPARTYDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;

class EditCounterpartyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditCounterpartyDialog(int counterpartyId, QWidget *parent = nullptr);
    ~EditCounterpartyDialog() = default;

private slots:
    void saveCounterparty();
    void validateForm();

private:
    void setupUI();
    void loadCounterpartyData();

    int counterpartyId_;
    
    // Элементы формы
    QLineEdit *nameEdit_;
    QLineEdit *innEdit_;
    QLineEdit *kppEdit_;
    QLineEdit *addressEdit_;
    QLineEdit *phoneEdit_;
    QLineEdit *emailEdit_;
    QPushButton *saveButton_;
};

#endif // EDITCOUNTERPARTYDIALOG_H