set(TARGET_NAME qtadvancedcss)

# Source files
set(SOURCES
    QmlStyleUrlInterceptor.cpp
    QtAdvancedStylesheet.cpp
)

# Header files
set(HEADERS
    acss_globals.h
    QmlStyleUrlInterceptor.h
    QtAdvancedStylesheet.h
)

# Create library
if(ACSS_BUILD_STATIC)
    add_library(${TARGET_NAME} STATIC ${SOURCES} ${HEADERS})
    target_compile_definitions(${TARGET_NAME} PUBLIC ACSS_STATIC)
else()
    add_library(${TARGET_NAME} SHARED ${SOURCES} ${HEADERS})
    target_compile_definitions(${TARGET_NAME} PRIVATE ACSS_SHARED_EXPORT)
endif()

# Add alias for consistent usage
add_library(QtAdvancedStylesheets::${TARGET_NAME} ALIAS ${TARGET_NAME})

# Set target properties
set_target_properties(${TARGET_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME qtadvancedcss
    DEBUG_POSTFIX d
)

# Link Qt libraries
target_link_libraries(${TARGET_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Svg
)

# Include directories
target_include_directories(${TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Compiler warnings
target_compile_definitions(${TARGET_NAME} PRIVATE QT_DEPRECATED_WARNINGS)

if(MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

# Installation
install(TARGETS ${TARGET_NAME}
    EXPORT QtAdvancedStylesheetsTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${HEADERS}
    DESTINATION include
)
