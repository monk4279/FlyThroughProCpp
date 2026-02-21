cmake_minimum_required(VERSION 3.16)

project(flythrough_pro_cpp)

# Add module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Enable Qt MOC (Meta-Object Compiler)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Enforce C++17 (Required by QGIS 3.28+)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------
# Qt5
# ---------------------------------------------------------------
find_package(Qt5 COMPONENTS Core Gui Widgets Xml Network REQUIRED)

# ---------------------------------------------------------------
# QGIS - locate headers and libraries
#
# Priority (first non-empty wins):
#   1. -DQGIS_INCLUDE_DIR / -DQGIS_LIBRARY_DIR passed to cmake
#   2. OSGEO4W_ROOT environment variable
#   3. Hard-coded fallback paths
# ---------------------------------------------------------------

# Resolve OSGEO4W root
if(NOT OSGEO4W_ROOT)
  if(DEFINED ENV{OSGEO4W_ROOT})
    set(OSGEO4W_ROOT "$ENV{OSGEO4W_ROOT}")
  else()
    set(OSGEO4W_ROOT "C:/OSGeo4W")
  endif()
endif()
message(STATUS "OSGEO4W_ROOT = ${OSGEO4W_ROOT}")

# --- Headers ---
if(NOT QGIS_INCLUDE_DIR)
  # Try known OSGeo4W sub-paths in order
  foreach(_candidate
      "${OSGEO4W_ROOT}/apps/qgis-ltr-dev/include"
      "${OSGEO4W_ROOT}/apps/qgis-ltr/include"
      "${OSGEO4W_ROOT}/apps/qgis/include"
      "${OSGEO4W_ROOT}/include"
  )
    if(EXISTS "${_candidate}/qgsapplication.h")
      set(QGIS_INCLUDE_DIR "${_candidate}")
      break()
    endif()
  endforeach()
endif()

if(NOT QGIS_INCLUDE_DIR OR NOT EXISTS "${QGIS_INCLUDE_DIR}/qgsapplication.h")
  message(FATAL_ERROR
    "Cannot find QGIS headers (qgsapplication.h).\n"
    "Pass -DQGIS_INCLUDE_DIR=<path> to cmake or set OSGEO4W_ROOT.\n"
    "Tried OSGEO4W_ROOT = ${OSGEO4W_ROOT}"
  )
endif()
message(STATUS "QGIS_INCLUDE_DIR = ${QGIS_INCLUDE_DIR}")

# --- Libraries ---
if(NOT QGIS_LIBRARY_DIR)
  foreach(_candidate
      "${OSGEO4W_ROOT}/apps/qgis-ltr-dev/lib"
      "${OSGEO4W_ROOT}/apps/qgis-ltr/lib"
      "${OSGEO4W_ROOT}/apps/qgis/lib"
      "${OSGEO4W_ROOT}/lib"
  )
    if(EXISTS "${_candidate}/qgis_core.lib")
      set(QGIS_LIBRARY_DIR "${_candidate}")
      break()
    endif()
  endforeach()
endif()

if(NOT QGIS_LIBRARY_DIR OR NOT EXISTS "${QGIS_LIBRARY_DIR}/qgis_core.lib")
  message(FATAL_ERROR
    "Cannot find QGIS libraries (qgis_core.lib).\n"
    "Pass -DQGIS_LIBRARY_DIR=<path> to cmake or set OSGEO4W_ROOT.\n"
    "Tried OSGEO4W_ROOT = ${OSGEO4W_ROOT}"
  )
endif()
message(STATUS "QGIS_LIBRARY_DIR = ${QGIS_LIBRARY_DIR}")

# Resolve actual library paths
find_library(QGIS_CORE_LIBRARY NAMES qgis_core HINTS "${QGIS_LIBRARY_DIR}" NO_DEFAULT_PATH)
find_library(QGIS_GUI_LIBRARY  NAMES qgis_gui  HINTS "${QGIS_LIBRARY_DIR}" NO_DEFAULT_PATH)
find_library(QGIS_3D_LIBRARY   NAMES qgis_3d   HINTS "${QGIS_LIBRARY_DIR}" NO_DEFAULT_PATH)

message(STATUS "QGIS_CORE_LIBRARY = ${QGIS_CORE_LIBRARY}")
message(STATUS "QGIS_GUI_LIBRARY  = ${QGIS_GUI_LIBRARY}")
message(STATUS "QGIS_3D_LIBRARY   = ${QGIS_3D_LIBRARY}")

if(NOT QGIS_CORE_LIBRARY)
  message(FATAL_ERROR "qgis_core.lib not found in ${QGIS_LIBRARY_DIR}")
endif()
if(NOT QGIS_GUI_LIBRARY)
  message(FATAL_ERROR "qgis_gui.lib not found in ${QGIS_LIBRARY_DIR}")
endif()

# ---------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Xml_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${QGIS_INCLUDE_DIR}
    ${QGIS_INCLUDE_DIR}/qgis
    ${QGIS_INCLUDE_DIR}/3d
)

# ---------------------------------------------------------------
# Source files
# ---------------------------------------------------------------
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

# ---------------------------------------------------------------
# Target
# ---------------------------------------------------------------
add_library(flythrough_pro_cpp SHARED ${SRCS} ${HDRS})

target_link_libraries(flythrough_pro_cpp
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Xml
    Qt5::Network
    ${QGIS_CORE_LIBRARY}
    ${QGIS_GUI_LIBRARY}
)

# Link 3D library only if found (optional)
if(QGIS_3D_LIBRARY)
  target_link_libraries(flythrough_pro_cpp ${QGIS_3D_LIBRARY})
endif()

target_compile_definitions(flythrough_pro_cpp PRIVATE
    FLYTHROUGH_PRO_CPP_LIBRARY
    _USE_MATH_DEFINES
    _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
)
