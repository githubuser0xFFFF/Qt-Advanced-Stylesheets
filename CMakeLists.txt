cmake_minimum_required(VERSION 3.16)

project(QtAdvancedStylesheets VERSION 1.0.5 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt specific settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Build type options
option(ACSS_BUILD_STATIC "Build as static library" OFF)
option(ACSS_BUILD_EXAMPLES "Build examples" ON)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets 
    Qml 
    Svg
)

# Installation settings
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/installed" CACHE PATH "Install prefix" FORCE)
endif()

# Add subdirectories
add_subdirectory(src)

if(ACSS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
install(EXPORT QtAdvancedStylesheetsTargets
    FILE QtAdvancedStylesheetsTargets.cmake
    NAMESPACE QtAdvancedStylesheets::
    DESTINATION lib/cmake/QtAdvancedStylesheets
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/QtAdvancedStylesheetsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/QtAdvancedStylesheetsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/QtAdvancedStylesheetsConfig.cmake"
    INSTALL_DESTINATION lib/cmake/QtAdvancedStylesheets
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/QtAdvancedStylesheetsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/QtAdvancedStylesheetsConfigVersion.cmake"
    DESTINATION lib/cmake/QtAdvancedStylesheets
)
