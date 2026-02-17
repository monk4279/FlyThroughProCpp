cmake_minimum_required(VERSION 3.16)

project(flythrough_pro_cpp)

# Add module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Enable Qt MOC (Meta-Object Compiler)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find QGIS and Qt dependencies
find_package(Qt5 COMPONENTS Core Gui Widgets Xml Network 3DCore 3DRender 3DInput 3DLogic 3DExtras REQUIRED)
find_package(QGIS REQUIRED)

# Includes
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Xml_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt53DCore_INCLUDE_DIRS}
    ${Qt53DRender_INCLUDE_DIRS}
    ${Qt53DInput_INCLUDE_DIRS}
    ${Qt53DLogic_INCLUDE_DIRS}
    ${Qt53DExtras_INCLUDE_DIRS}
    ${QGIS_INCLUDE_DIR}
    ${QGIS_INCLUDE_DIR}/3d
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

# Create shared library for the plugin
add_library(flythrough_pro_cpp SHARED ${SRCS} ${HDRS})

# Link libraries
target_link_libraries(flythrough_pro_cpp
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Xml
    Qt5::Network
    Qt5::3DCore
    Qt5::3DRender
    Qt5::3DInput
    Qt5::3DLogic
    Qt5::3DExtras
    ${QGIS_CORE_LIBRARY}
    ${QGIS_GUI_LIBRARY}
    ${QGIS_3D_LIBRARY}
)

# Definitions
target_compile_definitions(flythrough_pro_cpp PRIVATE FLYTHROUGH_PRO_CPP_LIBRARY)
