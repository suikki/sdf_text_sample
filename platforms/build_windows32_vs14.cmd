@echo off
setlocal
set PROJECT_DIR=%~dp0..
set BUILD_DIR=%PROJECT_DIR%\build\windows32_vs14
set CMAKE_EXE=cmake.exe

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%" || goto error
pushd "%BUILD_DIR%" || goto error

:: Configure cmake.
call %CMAKE_EXE% -G "Visual Studio 14 2015" "%PROJECT_DIR%" || goto error

:: Build the application.
call %CMAKE_EXE% --build . --config Debug || goto error
call %CMAKE_EXE% --build . --config Release || goto error

popd
goto end

:error
echo Build failed!
exit /b 1

:end
