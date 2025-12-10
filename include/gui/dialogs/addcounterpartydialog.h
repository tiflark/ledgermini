#ifndef ADDCOUNTERPARTYDIALOG_H
#define ADDCOUNTERPARTYDIALOG_H
#include <QDialog>
#include <QLineEdit>

class AddCounterpartyDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddCounterpartyDialog(QWidget *parent = nullptr);
private slots:
    void saveCounterparty();
private:
    QLineEdit *nameEdit;
    QLineEdit *innEdit;
    void setupUI();
};
#endif