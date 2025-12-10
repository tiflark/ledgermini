#include "gui/mainwindow.h"
#include "core/database.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QDebug>
#include "gui/dialogs/addcounterpartydialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Настройка приложения
    app.setApplicationName("LedgerMini");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("YourCompany");
    
    // Установка стиля (опционально)
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Установка глобального стиля
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(25, 25, 25));
    palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53, 53, 53));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(palette);
    
    // Загрузка стилей из файла (опционально)
    QFile styleFile(":/styles/darkstyle.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
    }
    
    // Создание и отображение главного окна
    MainWindow window;
    window.show();
    
    return app.exec();
}