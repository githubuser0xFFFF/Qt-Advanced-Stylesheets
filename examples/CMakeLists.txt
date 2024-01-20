cmake_minimum_required(VERSION 3.28)
project(Examples)

# exporter
add_executable(exporter exporter/exporter.cpp)
target_link_libraries(exporter PRIVATE Qt5::Core Qt5::Widgets Qt5::Gui ACSS::QtAdvancedStylesheet)
target_compile_definitions(exporter PRIVATE "STYLES_DIR=${ACS_STYLE_DIRECTORY}")

# full_features
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/full_features)

set(HEADERS 
    full_features/mainwindow.h)

set(SOURCES 
    full_features/mainwindow.cpp)

set(UI
full_features/mainwindow.ui)

set(QRC
    full_features/full_features.qrc)

add_executable(full_features full_features/main.cpp ${HEADERS} ${SOURCES} ${QRC})
target_link_libraries(full_features PRIVATE Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Qml Qt5::QuickWidgets ACSS::QtAdvancedStylesheet)
target_compile_definitions(full_features PRIVATE "STYLES_DIR=${ACS_STYLE_DIRECTORY}")