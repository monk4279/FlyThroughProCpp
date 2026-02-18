# Find QGIS
# ~~~~~~~~~
# Copyright (c) 2007, Martin Dobias <wonder.sk at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for QGIS library
#
# If it's found it sets QGIS_FOUND to TRUE
#
#Variables defined:
#    QGIS_FOUND - If QGIS library is found
#    QGIS_CORE_LIBRARY - The QGIS Core library file
#    QGIS_GUI_LIBRARY - The QGIS Gui library file
#    QGIS_ANALYSIS_LIBRARY - The QGIS Analysis library file
#    QGIS_3D_LIBRARY - The QGIS 3D library file
#    QGIS_INCLUDE_DIR - The QGIS include directory
#    QGIS_PLUGIN_DIR - The directory where dependencies for plugins are stored

# Reset
set(QGIS_FOUND FALSE)
set(QGIS_INCLUDE_DIR QGIS_INCLUDE_DIR-NOTFOUND)
set(QGIS_CORE_LIBRARY QGIS_CORE_LIBRARY-NOTFOUND)
set(QGIS_GUI_LIBRARY QGIS_GUI_LIBRARY-NOTFOUND)
set(QGIS_3D_LIBRARY QGIS_3D_LIBRARY-NOTFOUND)

# Look for header files
find_path(QGIS_INCLUDE_DIR NAMES qgsapplication.h
  PATHS
    /usr/include/qgis
    /usr/local/include/qgis
    /usr/include/qgis/qt4
    "$ENV{LIB_DIR}/include/qgis"
    "$ENV{LIB_DIR}/include"
    "$ENV{QGIS_DIR}/include/qgis"
    "$ENV{QGIS_DIR}/include"
    "${QGIS_DIR}/include/qgis"
    "${QGIS_DIR}/include"
    "$ENV{OSGEO4W_ROOT}/include"
    "$ENV{OSGEO4W_ROOT}/apps/qgis/include"
    "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr-dev/include"
    "C:/Program Files/QGIS 3.28.3/include"
    "C:/Program Files/QGIS 3.28.3/apps/qgis/include"
    "C:/Program Files/QGIS/include"
    "C:/Program Files/QGIS/apps/qgis/include"
    /opt/homebrew/opt/qgis/include
    /usr/local/opt/qgis/include
    /Applications/QGIS.app/Contents/Frameworks/qgis_core.framework/Headers
    /Applications/QGIS.app/Contents/Frameworks/qgis_gui.framework/Headers
    /Applications/QGIS-LTR.app/Contents/Frameworks/qgis_core.framework/Headers
    /Applications/QGIS-LTR.app/Contents/Frameworks/qgis_gui.framework/Headers
)

# Add OSGeo4W qgis-ltr-dev paths explicitly
if(NOT QGIS_INCLUDE_DIR AND EXISTS "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr-dev/include")
  set(QGIS_INCLUDE_DIR "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr-dev/include")
endif()

# Look for libraries
# Core
find_library(QGIS_CORE_LIBRARY 
  NAMES qgis_core qgis_core3 qgis_core.dll
  PATHS
    "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr-dev/lib"
    "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr-dev/bin"
    "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr/lib"
    "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr/bin"
    "$ENV{OSGEO4W_ROOT}/apps/qgis/lib"
    "$ENV{OSGEO4W_ROOT}/apps/qgis/bin"
    "$ENV{OSGEO4W_ROOT}/lib"
    "$ENV{OSGEO4W_ROOT}/bin"
    "$ENV{LIB_DIR}/lib"
    "$ENV{QGIS_DIR}/lib"
    "${QGIS_DIR}/lib"
    "C:/Program Files/QGIS 3.28.3/lib"
    "C:/Program Files/QGIS 3.28.3/bin"
    "C:/Program Files/QGIS/lib"
    "C:/Program Files/QGIS/bin"
    /usr/lib
    /usr/local/lib
    /opt/homebrew/opt/qgis/lib
    /usr/local/opt/qgis/lib
    /Applications/QGIS.app/Contents/Frameworks
    /Applications/QGIS-LTR.app/Contents/Frameworks
)

# Gui
find_library(QGIS_GUI_LIBRARY
  NAMES qgis_gui qgis_gui3 qgis_gui.dll
  PATHS
    "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr-dev/lib"
    "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr-dev/bin"
    "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr/lib"
    "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr/bin"
    "$ENV{OSGEO4W_ROOT}/apps/qgis/lib"
    "$ENV{OSGEO4W_ROOT}/apps/qgis/bin"
    "$ENV{OSGEO4W_ROOT}/lib"
    "$ENV{OSGEO4W_ROOT}/bin"
    "$ENV{LIB_DIR}/lib"
    "$ENV{QGIS_DIR}/lib"
    "${QGIS_DIR}/lib"
    "C:/Program Files/QGIS 3.28.3/lib"
    "C:/Program Files/QGIS 3.28.3/bin"
    "C:/Program Files/QGIS/lib"
    "C:/Program Files/QGIS/bin"
    /usr/lib
    /usr/local/lib
    /opt/homebrew/opt/qgis/lib
    /usr/local/opt/qgis/lib
    /Applications/QGIS.app/Contents/Frameworks
    /Applications/QGIS-LTR.app/Contents/Frameworks
)

# 3D
find_library(QGIS_3D_LIBRARY
  NAMES qgis_3d qgis_3d3 qgis_3d.dll
  PATHS
    "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr-dev/lib"
    "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr-dev/bin"
    "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr/lib"
    "$ENV{OSGEO4W_ROOT}/apps/qgis-ltr/bin"
    "$ENV{OSGEO4W_ROOT}/apps/qgis/lib"
    "$ENV{OSGEO4W_ROOT}/apps/qgis/bin"
    "$ENV{OSGEO4W_ROOT}/lib"
    "$ENV{OSGEO4W_ROOT}/bin"
    "$ENV{LIB_DIR}/lib"
    "$ENV{QGIS_DIR}/lib"
    "${QGIS_DIR}/lib"
    "C:/Program Files/QGIS 3.28.3/lib"
    "C:/Program Files/QGIS 3.28.3/bin"
    "C:/Program Files/QGIS/lib"
    "C:/Program Files/QGIS/bin"
    /usr/lib
    /usr/local/lib
    /opt/homebrew/opt/qgis/lib
    /usr/local/opt/qgis/lib
    /Applications/QGIS.app/Contents/Frameworks
    /Applications/QGIS-LTR.app/Contents/Frameworks
)

# --- Debug Output ---
message(STATUS "--- QGIS Discovery Debug ---")
if(QGIS_CORE_LIBRARY AND QGIS_GUI_LIBRARY AND QGIS_INCLUDE_DIR)
  set(QGIS_FOUND TRUE)
  message(STATUS "QGIS FOUND!")
else()
  message(FATAL_ERROR "Could not find QGIS")
endif()

message(STATUS "QGIS_INCLUDE_DIR: ${QGIS_INCLUDE_DIR}")
message(STATUS "QGIS_CORE_LIBRARY: ${QGIS_CORE_LIBRARY}")
message(STATUS "QGIS_GUI_LIBRARY: ${QGIS_GUI_LIBRARY}")
message(STATUS "QGIS_3D_LIBRARY: ${QGIS_3D_LIBRARY}")
message(STATUS "Env QGIS_DIR: $ENV{QGIS_DIR}")

if (QGIS_FOUND)
   if (NOT QGIS_FIND_QUIETLY)
      message(STATUS "Found QGIS: ${QGIS_CORE_LIBRARY}")
   endif ()
else ()
   if (QGIS_FIND_REQUIRED)
      message(STATUS "QGIS_INCLUDE_DIR: ${QGIS_INCLUDE_DIR}")
      message(STATUS "QGIS_CORE_LIBRARY: ${QGIS_CORE_LIBRARY}")
      message(STATUS "QGIS_GUI_LIBRARY: ${QGIS_GUI_LIBRARY}")
      message(STATUS "Env QGIS_DIR: $ENV{QGIS_DIR}")
      message(FATAL_ERROR "Could not find QGIS")
   endif ()
endif ()
