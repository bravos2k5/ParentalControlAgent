#include "DeviceInfo.h"
#include "Logger.h"
#include <windows.h>
#include <sstream>
#include <iomanip>

std::string DeviceInfo::getDeviceId() {
    return generateMachineGuid();
}

std::string DeviceInfo::getDeviceName() {
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);
    
    if (GetComputerNameA(computerName, &size)) {
        return std::string(computerName);
    }
    
    LOG_WARNING("Failed to get computer name, using default");
    return "Unknown-PC";
}

std::string DeviceInfo::generateMachineGuid() {
    HKEY hKey;
    std::string machineGuid = "unknown-device-id";
    
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                      "SOFTWARE\\Microsoft\\Cryptography", 
                      0, 
                      KEY_READ | KEY_WOW64_64KEY, 
                      &hKey) == ERROR_SUCCESS) {
        char buffer[256];
        DWORD bufferSize = sizeof(buffer);
        DWORD type = REG_SZ;
        
        if (RegQueryValueExA(hKey, "MachineGuid", NULL, &type, 
                            (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS) {
            machineGuid = std::string(buffer);
        }
        
        RegCloseKey(hKey);
    }
    
    LOG_INFO("Device ID: " + machineGuid);
    return machineGuid;
}
