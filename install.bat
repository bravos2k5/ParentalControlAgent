@echo off
:: Parental Control Agent - Installation Script
:: This script creates a scheduled task to run the agent at startup
:: Must be run as Administrator

setlocal enabledelayedexpansion

:: Check for admin privileges
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo ============================================
    echo ERROR: Administrator privileges required!
    echo Please right-click and "Run as administrator"
    echo ============================================
    pause
    exit /b 1
)

:: Get the directory where this script is located
:: You can replace this with the actual path to the executable if needed
set "SCRIPT_DIR=%~dp0"
set "EXE_PATH=%SCRIPT_DIR%build\ParentalControlAgent.exe"

:: Check if executable exists
if not exist "%EXE_PATH%" (
    echo ============================================
    echo ERROR: ParentalControlAgent.exe not found!
    echo Expected location: %EXE_PATH%
    echo Please build the project first.
    echo ============================================
    pause
    exit /b 1
)

echo ============================================
echo Parental Control Agent - Installer
echo ============================================
echo.
echo Executable: %EXE_PATH%
echo.

:: Delete existing task if it exists
echo Removing existing scheduled task (if any)...
schtasks /delete /tn "ParentalControlAgent" /f >nul 2>&1

:: Create scheduled task to run at logon with highest privileges
echo Creating scheduled task...
schtasks /create /tn "ParentalControlAgent" /tr "\"%EXE_PATH%\"" /sc onlogon /rl highest /f

if %errorlevel% equ 0 (
    echo.
    echo ============================================
    echo SUCCESS: Installation complete!
    echo ============================================
    echo.
    echo The Parental Control Agent will now:
    echo - Start automatically when any user logs in
    echo - Run with administrator privileges
    echo.
    echo To uninstall, run: uninstall.bat
    echo To start now, run the executable or restart.
    echo.
) else (
    echo.
    echo ============================================
    echo ERROR: Failed to create scheduled task!
    echo ============================================
)

pause
