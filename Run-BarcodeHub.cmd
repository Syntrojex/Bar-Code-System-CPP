@echo off
setlocal
set "APP=%~dp0build\portable\BarcodeHub\BarcodeHub.exe"
if not exist "%APP%" (
    echo Build the application first:
    echo powershell -ExecutionPolicy Bypass -File scripts\build-windows.ps1
    pause
    exit /b 1
)
start "" "%APP%"
