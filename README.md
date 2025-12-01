# Parental Control Agent

A Windows overlay application for parental control, built with C++ (MinGW64), CMake, and IXWebSocket.

## Features

- **Fullscreen Overlay**: Blocks user access until authentication
- **WebSocket Communication**: Real-time communication with control server
- **Timer Management**: Handles granted access time, block warnings, and shutdown countdowns
- **Emergency Mode**: Offline access with emergency password when server is unavailable
- **Single Instance**: Mutex-based enforcement of single application instance
- **Dark Theme UI**: Clean interface with RGB(30, 30, 30) background

## Message Protocol

### Received from Server:
- `GRANTED:{X}` - Hide overlay, allow access for X seconds
- `BLOCK:{Y}` - Show notification, block after Y seconds (minimum 5 seconds)
- `SHUTDOWN:{Z}` - Force shutdown after Z seconds
- `DENIED` - Wrong password notification

### Sent to Server:
- `BLOCKED` - Sent when user gets blocked
- `PASSWORD:{password}` - Send password for authentication

## Headers

The application sends these headers when connecting:
- `X-Device-Id` - Unique machine identifier
- `X-Device-Name` - Computer name

## Emergency Mode

- Activated automatically when server connection fails
- Password: `emergency123`
- Grants 1 hour of access

## Building

### Prerequisites

1. **MinGW-w64** (64-bit)
2. **CMake** (3.16+)
3. **OpenSSL** (for TLS support)

### Build Steps

```powershell
# Create build directory
mkdir build
cd build

# Configure with CMake (adjust paths as needed)
cmake -G "MinGW Makefiles" -DOPENSSL_ROOT_DIR="C:/msys64/mingw64" ..

# Build
cmake --build . --config Release
```

### Using MSYS2

If using MSYS2, install dependencies:

```bash
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-openssl
```

## Configuration

The WebSocket endpoint is configured in `App.cpp`:
```cpp
const std::string App::WS_ENDPOINT = "wss://api.bravos.io.vn/ws/";
```

## Logging

Logs are written to `parental_control.log` in the same directory as the executable. No console output is generated.

## UI Controls

- **Password Input**: Enter authentication password
- **Unlock Button**: Submit password
- **Shutdown Button**: Shutdown the computer
- **Restart Button**: Restart the computer

## License

See LICENSE file.
