#ifndef MANAGETEMPLATESDIALOG_H
#define MANAGETEMPLATESDIALOG_H

#include <QDialog>
#include <QStandardItemModel>

class QTableView;
class QPushButton;
class QComboBox;

class ManageTemplatesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManageTemplatesDialog(QWidget *parent = nullptr);
    
private slots:
    void addTemplate();
    void editTemplate();
    void deleteTemplate();
    void useTemplate();
    void refreshTable();
    void onDoubleClick(const QModelIndex &index);

private:
    void setupUI();
    void loadTemplates();
    void setupFrequencyCombo(QComboBox *combo);
    
    QTableView *tableView;
    QStandardItemModel *model;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *useButton;
    QPushButton *refreshButton;
    QPushButton *closeButton;
};

#endif // MANAGETEMPLATESDIALOG_H