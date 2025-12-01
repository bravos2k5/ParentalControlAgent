@echo off
:: Parental Control Agent - Uninstallation Script
:: This script removes the scheduled task
:: Must be run as Administrator

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

echo ============================================
echo Parental Control Agent - Uninstaller
echo ============================================
echo.

:: Stop any running instance
echo Stopping running instances...
taskkill /f /im ParentalControlAgent.exe >nul 2>&1

:: Delete scheduled task
echo Removing scheduled task...
schtasks /delete /tn "ParentalControlAgent" /f

if %errorlevel% equ 0 (
    echo.
    echo ============================================
    echo SUCCESS: Uninstallation complete!
    echo ============================================
    echo.
    echo The scheduled task has been removed.
    echo The executable files remain in place.
    echo.
) else (
    echo.
    echo ============================================
    echo WARNING: Scheduled task may not exist
    echo ============================================
)

pause
