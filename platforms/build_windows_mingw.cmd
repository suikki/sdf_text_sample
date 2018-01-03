@echo off
setlocal

:: For overriding the mingw path:
::set MINGW_HOME=C:\mingw
::set PATH=%MINGW_HOME%\bin;%PATH%


set PROJECT_DIR=%~dp0\..
set BUILD_DIR=%PROJECT_DIR%\build\windows_mingw

:: Setup the build environment.
if exist "%MINGW_HOME%\bin\mingw32-make.exe" goto mingw_ok
echo MinGW not found or location not defined (MINGW_HOME="%MINGW_HOME%")
goto error
:mingw_ok
set CMAKE_EXE=cmake.exe
set MAKE_EXE=%MINGW_HOME%\bin\mingw32-make.exe


:: Debug version.
if not exist "%BUILD_DIR%_debug" mkdir "%BUILD_DIR%_debug" || goto error
pushd "%BUILD_DIR%_debug" || goto error
call %CMAKE_EXE% -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE:STRING=Debug VERBOSE=0 "%PROJECT_DIR%" || goto error
call %MAKE_EXE% || goto error
popd

:: Release version.
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%" || goto error
pushd "%BUILD_DIR%" || goto error
call %CMAKE_EXE% -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE:STRING=MinSizeRel VERBOSE=0 "%PROJECT_DIR%" || goto error
call %MAKE_EXE% || goto error
popd


goto end

:error
echo Build failed!
exit /b 1

:end
