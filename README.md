# Parental Control Agent

A Windows overlay application for parental control, built with C++ (MinGW64), CMake, IXWebSocket, and MbedTLS.

## Features

- **Fullscreen Overlay**: Blocks user access until authentication
- **WebSocket Communication**: Real-time communication with control server (TLS via MbedTLS)
- **Timer Management**: Handles granted access time and block countdowns
- **Emergency Mode**: Offline access with emergency password when server is unavailable
- **Single Instance**: Mutex-based enforcement of single application instance
- **Keyboard Blocking**: Blocks system shortcuts (Alt+Tab, Win key, etc.) when locked
- **Dark Theme UI**: Clean interface with dark background

## Message Protocol

### Received from Server:
- `GRANTED:{X}` - Hide overlay, allow access for X seconds
- `BLOCK:{X}` - Block user after X seconds (no notification during countdown)
- `SHUTDOWN:{X}` - Force shutdown after X seconds
- `DENIED` - Wrong password notification

### Sent to Server:
- `BLOCKED` - Sent when user gets blocked
- `PASSWORD:{password}` - Send password for authentication

## Headers

The application sends these headers when connecting:
- `X-Device-Id` - Unique machine identifier
- `X-Device-Name` - Computer name

## Emergency Mode

- Activated automatically when server connection fails after 60 seconds
- Password: `emergency123`
- Grants 1 hour of access

## Building

### Prerequisites

- **MinGW-w64** with GCC (64-bit)
- **CMake** 3.16 or higher

All other dependencies (IXWebSocket, MbedTLS) are automatically downloaded via CMake FetchContent.

### Build Steps

```powershell
# Clone the repository
git clone https://github.com/bravos2k5/ParentalControlAgent.git
cd ParentalControlAgent

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake -G "MinGW Makefiles" "-DCMAKE_POLICY_VERSION_MINIMUM=3.5" ..

# Build (use -j for parallel compilation)
mingw32-make -j4

# The executable will be at build/ParentalControlAgent.exe
```

### Alternative: Using Visual Studio

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Configuration

The WebSocket endpoint is configured in `src/App.cpp`:
```cpp
const std::string App::WS_ENDPOINT = "wss://example.yourdomain.com/ws/";
```

The emergency password is also in `src/App.cpp`:
```cpp
const std::string App::EMERGENCY_PASSWORD = "emergency123";
```

## Logging

Logs are written to `parental_control.log` in the same directory as the executable.

## Project Structure

```
ParentalControlAgent/
├── CMakeLists.txt          # Build configuration
├── README.md
├── LICENSE
├── install.bat             # Install as startup task (run as admin)
├── uninstall.bat           # Remove startup task (run as admin)
├── resources/
│   └── app.rc              # Windows resources
├── src/
│   ├── main.cpp            # Entry point
│   ├── App.cpp/h           # Main application logic
│   ├── OverlayWindow.cpp/h # UI overlay window
│   ├── WebSocketClient.cpp/h # WebSocket communication
│   ├── TimerManager.cpp/h  # Timer management
│   ├── DeviceInfo.cpp/h    # Device identification
│   └── Logger.cpp/h        # Logging utility
└── build/                  # Build output (generated)
```

## Installation

After building, you can install the agent to run automatically at startup:

```powershell
# Run as Administrator
.\install.bat
```

This creates a Windows scheduled task that:
- Starts the agent when any user logs in
- Runs with administrator privileges

To uninstall:
```powershell
# Run as Administrator
.\uninstall.bat
```

## License

See LICENSE file.
