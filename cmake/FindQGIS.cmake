# Find QGIS
# ~~~~~~~~~
# Copyright (c) 2007, Martin Dobias <wonder.sk at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
#
# CMake module to search for QGIS library
# Variables defined:
#   QGIS_FOUND
#   QGIS_CORE_LIBRARY
#   QGIS_GUI_LIBRARY
#   QGIS_3D_LIBRARY
#   QGIS_INCLUDE_DIR
#   QGIS_PLUGIN_DIR

# --- Honor explicit hints passed via -DQGIS_INCLUDE_DIR and -DQGIS_LIBRARY_DIR ---
# If the caller already set these, use them directly without searching.

if(QGIS_INCLUDE_DIR AND EXISTS "${QGIS_INCLUDE_DIR}/qgsapplication.h")
  message(STATUS "Using caller-provided QGIS_INCLUDE_DIR: ${QGIS_INCLUDE_DIR}")
else()
  # Search for header directory
  find_path(QGIS_INCLUDE_DIR
    NAMES qgsapplication.h
    HINTS
      "${QGIS_INCLUDE_DIR}"
      "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr-dev/include"
      "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr/include"
      "$ENV{OSGEO4W_ROOT}/apps/qgis/include"
      "$ENV{OSGEO4W_ROOT}/include"
      "$ENV{LIB_DIR}/include/qgis"
      "$ENV{LIB_DIR}/include"
      "$ENV{QGIS_DIR}/include"
    PATHS
      /usr/include/qgis
      /usr/local/include/qgis
      "C:/Program Files/QGIS 3.28.3/apps/qgis/include"
      "C:/Program Files/QGIS/apps/qgis/include"
      /Applications/QGIS.app/Contents/Frameworks/qgis_core.framework/Headers
      /Applications/QGIS-LTR.app/Contents/Frameworks/qgis_core.framework/Headers
  )
endif()

# --- Core library ---
if(QGIS_LIBRARY_DIR AND EXISTS "${QGIS_LIBRARY_DIR}")
  find_library(QGIS_CORE_LIBRARY
    NAMES qgis_core
    HINTS "${QGIS_LIBRARY_DIR}"
    NO_DEFAULT_PATH
  )
else()
  find_library(QGIS_CORE_LIBRARY
    NAMES qgis_core
    HINTS
      "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr-dev/lib"
      "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr/lib"
      "$ENV{OSGEO4W_ROOT}/apps/qgis/lib"
      "$ENV{OSGEO4W_ROOT}/lib"
      "$ENV{LIB_DIR}/lib"
      "$ENV{QGIS_DIR}/lib"
    PATHS
      /usr/lib /usr/local/lib
      /opt/homebrew/opt/qgis/lib
      /Applications/QGIS.app/Contents/Frameworks
      /Applications/QGIS-LTR.app/Contents/Frameworks
  )
endif()

# --- GUI library ---
if(QGIS_LIBRARY_DIR AND EXISTS "${QGIS_LIBRARY_DIR}")
  find_library(QGIS_GUI_LIBRARY
    NAMES qgis_gui
    HINTS "${QGIS_LIBRARY_DIR}"
    NO_DEFAULT_PATH
  )
else()
  find_library(QGIS_GUI_LIBRARY
    NAMES qgis_gui
    HINTS
      "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr-dev/lib"
      "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr/lib"
      "$ENV{OSGEO4W_ROOT}/apps/qgis/lib"
      "$ENV{OSGEO4W_ROOT}/lib"
      "$ENV{LIB_DIR}/lib"
      "$ENV{QGIS_DIR}/lib"
    PATHS
      /usr/lib /usr/local/lib
      /opt/homebrew/opt/qgis/lib
      /Applications/QGIS.app/Contents/Frameworks
      /Applications/QGIS-LTR.app/Contents/Frameworks
  )
endif()

# --- 3D library ---
if(QGIS_LIBRARY_DIR AND EXISTS "${QGIS_LIBRARY_DIR}")
  find_library(QGIS_3D_LIBRARY
    NAMES qgis_3d
    HINTS "${QGIS_LIBRARY_DIR}"
    NO_DEFAULT_PATH
  )
else()
  find_library(QGIS_3D_LIBRARY
    NAMES qgis_3d
    HINTS
      "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr-dev/lib"
      "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr/lib"
      "$ENV{OSGEO4W_ROOT}/apps/qgis/lib"
      "$ENV{OSGEO4W_ROOT}/lib"
      "$ENV{LIB_DIR}/lib"
      "$ENV{QGIS_DIR}/lib"
    PATHS
      /usr/lib /usr/local/lib
      /opt/homebrew/opt/qgis/lib
      /Applications/QGIS.app/Contents/Frameworks
      /Applications/QGIS-LTR.app/Contents/Frameworks
  )
endif()

# --- Debug Output ---
message(STATUS "--- QGIS Discovery Debug ---")
message(STATUS "QGIS_INCLUDE_DIR:   ${QGIS_INCLUDE_DIR}")
message(STATUS "QGIS_CORE_LIBRARY:  ${QGIS_CORE_LIBRARY}")
message(STATUS "QGIS_GUI_LIBRARY:   ${QGIS_GUI_LIBRARY}")
message(STATUS "QGIS_3D_LIBRARY:    ${QGIS_3D_LIBRARY}")
message(STATUS "QGIS_LIBRARY_DIR:   ${QGIS_LIBRARY_DIR}")
message(STATUS "Env OSGEO4W_ROOT:   $ENV{OSGEO4W_ROOT}")

# --- Determine FOUND ---
if(QGIS_CORE_LIBRARY AND QGIS_GUI_LIBRARY AND QGIS_INCLUDE_DIR)
  set(QGIS_FOUND TRUE)
  if(NOT QGIS_FIND_QUIETLY)
    message(STATUS "Found QGIS: ${QGIS_CORE_LIBRARY}")
  endif()
else()
  set(QGIS_FOUND FALSE)
  if(QGIS_FIND_REQUIRED)
    message(FATAL_ERROR
      "Could not find QGIS.\n"
      "  QGIS_INCLUDE_DIR = ${QGIS_INCLUDE_DIR}\n"
      "  QGIS_CORE_LIBRARY = ${QGIS_CORE_LIBRARY}\n"
      "  QGIS_GUI_LIBRARY = ${QGIS_GUI_LIBRARY}\n"
      "  Hint: set OSGEO4W_ROOT env var or pass -DQGIS_INCLUDE_DIR and -DQGIS_LIBRARY_DIR to cmake"
    )
  endif()
endif()
