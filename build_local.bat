@echo off
setlocal

echo ==========================================
echo   FlyThrough Pro C++ - Local Build Script
echo ==========================================
echo.
echo NOTE: This script must be run from the "OSGeo4W Shell"!
echo.

REM 1. Check for standard QGIS paths
if "%OSGEO4W_ROOT%"=="" (
    echo [ERROR] OSGEO4W_ROOT is not defined. 
    echo Please run this script from the "OSGeo4W Shell".
    echo.
    pause
    exit /b 1
)

REM 2. Identify and setup Visual Studio
echo [INFO] Setting up Visual Studio environment...
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo [WARNING] Could not find vcvars64.bat automatically.
    echo Please open "x64 Native Tools Command Prompt" and run this script from there if Visual Studio is not found.
)

REM 3. Create Build Directory
if not exist build mkdir build
cd build

REM 4. Configure with CMake
echo.
echo [INFO] Configuring CMake...
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="%OSGEO4W_ROOT%/apps/qgis;%OSGEO4W_ROOT%/apps/Qt5" ..
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed.
    echo Ensure 'cmake' and 'ninja' are installed via OSGeo4W (run setup-x86_64.exe).
    pause
    exit /b 1
)

REM 5. Build
echo.
echo [INFO] Building...
ninja
if %errorlevel% neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b 1
)

echo.
echo ==========================================
echo [SUCCESS] Build Complete!
echo.
echo The plugin DLL is located at:
echo %CD%\flythrough_pro_cpp.dll
echo.
echo Please copy this file to:
echo %OSGEO4W_ROOT%\apps\qgis\plugins
echo ==========================================
pause
