#pragma once

#include <string>

class DeviceInfo {
public:
    static std::string getDeviceId();
    static std::string getDeviceName();
    
private:
    static std::string generateMachineGuid();
};
