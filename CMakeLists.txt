cmake_minimum_required(VERSION 3.28)
project(QtAdvancedStylesheet)

# Options
option(ACS_BUILD_EXAMPLES "Generates examples" OFF)
option(ACS_BUILD_STATIC "Build static libraries" OFF)

if (ACS_BUILD_STATIC MATCHES ON)
    message(STATUS "Building static library")
    set(ACS_LIBRARY STATIC)
    set(ACS_COMPILE_DEFINITIONS ACSS_STATIC)
else ()
    message(STATUS "Building shared library")
    set(ACS_LIBRARY SHARED)
    set(ACS_COMPILE_DEFINITIONS ACSS_SHARED_EXPORT)
endif()

# Externals
find_package(Qt5 COMPONENTS Core Gui Widgets Qml QuickWidgets Svg REQUIRED)
set(CMAKE_AUTOMOC ON)


# Sources
set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/acss_globals.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/QmlStyleUrlInterceptor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/QtAdvancedStylesheet.h)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/QmlStyleUrlInterceptor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/QtAdvancedStylesheet.cpp)

add_library(qtadvancedcss SHARED ${HEADERS} ${SOURCES})
add_library(ACSS::QtAdvancedStylesheet ALIAS qtadvancedcss)

target_link_libraries(qtadvancedcss PUBLIC Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Qml Qt5::Svg)
message(STATUS "ACS_COMPILE_DEFINITIONS=${ACS_COMPILE_DEFINITIONS}")
target_compile_definitions(qtadvancedcss PRIVATE ${ACS_COMPILE_DEFINITIONS} ACSS_DEBUG_PRINT)
target_include_directories(qtadvancedcss PUBLIC 
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)


if(ACS_BUILD_EXAMPLES MATCHES ON)
    message(STATUS "Adding examples")
    set(ACS_STYLE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/styles)
    add_subdirectory(examples)
endif()