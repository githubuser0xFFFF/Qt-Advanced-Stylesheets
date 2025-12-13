set(TARGET_NAME exporter)

# Find additional Qt components
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets)

# Source files
set(SOURCES
    exporter.cpp
)

# Create executable
add_executable(${TARGET_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${TARGET_NAME}
    PRIVATE
        QtAdvancedStylesheets::qtadvancedcss
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
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

if(ACSS_BUILD_STATIC)
    target_compile_definitions(${TARGET_NAME} PRIVATE ACSS_STATIC)
endif()

# Set target properties
set_target_properties(${TARGET_NAME} PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)
