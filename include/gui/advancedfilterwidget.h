#ifndef ADVANCEDFILTERWIDGET_H
#define ADVANCEDFILTERWIDGET_H

#include <QWidget>
#include <QDate>

class QLineEdit;
class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QPushButton;
class QGroupBox;
class QCheckBox;

class AdvancedFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdvancedFilterWidget(QWidget *parent = nullptr);
    
    struct FilterOptions {
        QString textFilter;
        QString fieldFilter;
        QDate dateFrom;
        QDate dateTo;
        bool dateFilterEnabled;
        double amountFrom;
        double amountTo;
        bool amountFilterEnabled;
        int debitAccountId;
        int creditAccountId;
        int counterpartyId;
        bool useSavedFilter;
        QString savedFilterName;
    };
    
    FilterOptions getFilterOptions() const;
    void setFilterOptions(const FilterOptions &options);
    
    void clearFilters();
    void saveFilter(const QString &name);
    void loadFilter(const QString &name);
    QStringList getSavedFilters() const;
    
signals:
    void filterApplied();
    void filterSaved(const QString &name);
    void filterLoaded(const QString &name);

private slots:
    void onApplyClicked();
    void onClearClicked();
    void onSaveClicked();
    void onLoadClicked();

private:
    void setupUI();
    void loadAccounts();
    void loadCounterparties();
    void loadSavedFiltersList();
    void initConnections();  
    
    // Элементы фильтрации
    QLineEdit *searchEdit;
    QComboBox *fieldCombo;
    
    QGroupBox *dateGroup;
    QDateEdit *dateFromEdit;
    QDateEdit *dateToEdit;
    
    QGroupBox *amountGroup;
    QDoubleSpinBox *amountFromSpin;
    QDoubleSpinBox *amountToSpin;
    
    QComboBox *debitAccountCombo;
    QComboBox *creditAccountCombo;
    QComboBox *counterpartyCombo;
    
    QGroupBox *savedFilterGroup;
    QComboBox *savedFilterCombo;
    QPushButton *saveFilterButton;
    QPushButton *loadFilterButton;
    QPushButton *deleteFilterButton;
    
    QPushButton *applyButton;
    QPushButton *clearButton;
    
    // Хранилище фильтров (в реальном приложении - в базе данных)
    QMap<QString, FilterOptions> savedFilters;
};

#endif // ADVANCEDFILTERWIDGET_H