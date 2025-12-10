cmake_minimum_required(VERSION 3.16)
project(LedgerMini VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Настройка для Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Явно указываем Qt6
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets 
    Sql 
    PrintSupport 
    Charts
)

# Включение поддиректорий
add_subdirectory(src)

# Опции сборки
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()