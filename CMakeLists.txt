cmake_minimum_required(VERSION 3.16)

project(flythrough_pro_cpp)

# Add module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Enable Qt MOC (Meta-Object Compiler)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find QGIS and Qt dependencies
find_package(Qt5 COMPONENTS Core Gui Widgets Xml Network REQUIRED)
find_package(QGIS REQUIRED)

# Includes
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Xml_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${QGIS_INCLUDE_DIR}
)

# Source files
set(SRCS
    src/flythrough_plugin.cpp
    src/flythrough_dialog.cpp
    src/flythrough_core.cpp
)

set(HDRS
    src/flythrough_plugin.h
    src/flythrough_dialog.h
    src/flythrough_core.h
)

set(UIS
    resources/dialog.ui
)

# Handle UI files
# qt5_wrap_ui(UI_HDRS ${UIS}) # Uncomment when UI file exists

# Create shared library for the plugin
add_library(flythrough_pro_cpp SHARED ${SRCS} ${HDRS})

# Link libraries
target_link_libraries(flythrough_pro_cpp
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Xml
    Qt5::Network
    ${QGIS_CORE_LIBRARY}
    ${QGIS_GUI_LIBRARY}
)

# Definitions
target_compile_definitions(flythrough_pro_cpp PRIVATE FLYTHROUGH_PRO_CPP_LIBRARY)

# Initial metadata file copy (optional)
# configure_file(metadata.txt ${CMAKE_CURRENT_BINARY_DIR}/metadata.txt COPYONLY)
