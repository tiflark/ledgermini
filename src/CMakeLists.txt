# Явное указание исходных файлов
set(CORE_SOURCES
    core/database.cpp
    core/report_generator.cpp
    core/exportmanager.cpp
    core/validationrules.cpp
)

set(GUI_SOURCES
    gui/mainwindow.cpp
    gui/dialogs/addcounterpartydialog.cpp
    gui/dialogs/addtransactiondialog.cpp
    gui/dialogs/addeditaccountdialog.cpp
    gui/dialogs/editcounterpartydialog.cpp
    gui/dialogs/edittransactiondialog.cpp
    gui/reportwidget.cpp
    gui/accountcardwidget.cpp
    gui/tableactions.cpp
#   gui/searchwidget.cpp
    gui/dialogs/managetemplatesdialog.cpp
    gui/dialogs/edittemplatedialog.cpp
    gui/advancedfilterwidget.cpp
    gui/operationsjournalwidget.cpp
)

set(HEADER_FILES
    ../include/core/database.h
    ../include/core/report_generator.h
    ../include/gui/mainwindow.h
    ../include/gui/dialogs/addcounterpartydialog.h
    ../include/gui/dialogs/addtransactiondialog.h
    ../include/gui/dialogs/addeditaccountdialog.h
    ../include/gui/dialogs/editcounterpartydialog.h
    ../include/gui/dialogs/edittransactiondialog.h
    ../include/gui/reportwidget.h
    ../include/gui/accountcardwidget.h
    ../include/gui/tableactions.h
#   ../include/gui/searchwidget.h
    ../include/core/exportmanager.h
    ../include/core/validationrules.h
    ../include/gui/dialogs/managetemplatesdialog.h
    ../include/gui/dialogs/edittemplatedialog.h
    ../include/gui/advancedfilterwidget.h
    ../include/gui/operationsjournalwidget.h
)

# Основное приложение
add_executable(${PROJECT_NAME}  # Это "LedgerMini" из корневого CMakeLists.txt
    main.cpp
    ${CORE_SOURCES}
    ${GUI_SOURCES}
    ${HEADER_FILES}
)

# Связывание с библиотеками Qt
target_link_libraries(${PROJECT_NAME}  # Используем переменную ${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
    Qt6::Charts
)

# Включение директорий
target_include_directories(${PROJECT_NAME} PRIVATE  # Используем ${PROJECT_NAME}
    ../include
)

# Установка
install(TARGETS ${PROJECT_NAME} DESTINATION bin)  # Используем ${PROJECT_NAME}