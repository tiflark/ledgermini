#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QComboBox;

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = nullptr);
    
    QString getSearchText() const;
    QString getSearchField() const;
    
    void setPlaceholderText(const QString &text);
    void addSearchField(const QString &field, const QString &displayName);

signals:
    void searchRequested(const QString &text, const QString &field);

public slots:
    void clear();

private slots:
    void onSearchClicked();
    void onTextChanged(const QString &text);

private:
    void setupUI();
    
    QLineEdit *searchEdit;
    QComboBox *fieldCombo;
    QPushButton *searchButton;
    QPushButton *clearButton;
};

#endif // SEARCHWIDGET_H
