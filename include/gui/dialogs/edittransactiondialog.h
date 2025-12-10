#ifndef EDITTRANSACTIONDIALOG_H
#define EDITTRANSACTIONDIALOG_H

#include "addtransactiondialog.h"

class EditTransactionDialog : public AddTransactionDialog
{
    Q_OBJECT

public:
    explicit EditTransactionDialog(int transactionId, QWidget *parent = nullptr);

protected slots:
    void saveTransaction() override;  // Используем override

private:
    void loadTransactionData();
    
    int transactionId_;
};

#endif // EDITTRANSACTIONDIALOG_H