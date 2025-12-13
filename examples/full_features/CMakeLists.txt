set(TARGET_NAME full_features)

# Find additional Qt components
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets 
    QuickWidgets
    Qml
)

# Source files
set(SOURCES
    main.cpp
    mainwindow.cpp
)

# Header files
set(HEADERS
    mainwindow.h
)

# UI files
set(FORMS
    mainwindow.ui
)

# Resource files
set(RESOURCES
    full_features.qrc
)

# Create executable
add_executable(${TARGET_NAME}
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES}
)

# Link libraries
target_link_libraries(${TARGET_NAME}
    PRIVATE
        QtAdvancedStylesheets::qtadvancedcss
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::QuickWidgets
        Qt${QT_VERSION_MAJOR}::Qml
)

# Include directories
target_include_directories(${TARGET_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# Compiler definitions
target_compile_definitions(${TARGET_NAME}
    PRIVATE
        QT_DEPRECATED_WARNINGS
        STYLES_DIR=${CMAKE_SOURCE_DIR}/styles
)

# Set target properties
set_target_properties(${TARGET_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
