cmake_minimum_required(VERSION 3.16)
project(ParentalControlAgent VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows subsystem (no console window)
set(CMAKE_WIN32_EXECUTABLE ON)

include(FetchContent)

# Fetch MbedTLS first (use 2.x branch for ixwebsocket compatibility)
FetchContent_Declare(
    mbedtls
    GIT_REPOSITORY https://github.com/Mbed-TLS/mbedtls.git
    GIT_TAG v2.28.8
)
set(ENABLE_TESTING OFF CACHE BOOL "Disable MbedTLS tests")
set(ENABLE_PROGRAMS OFF CACHE BOOL "Disable MbedTLS programs")
FetchContent_MakeAvailable(mbedtls)

# Get MbedTLS directories
FetchContent_GetProperties(mbedtls SOURCE_DIR MBEDTLS_SRC_DIR)

# Pre-set MbedTLS variables so IXWebSocket's find_package succeeds
set(MBEDTLS_FOUND TRUE CACHE BOOL "")
set(MBEDTLS_INCLUDE_DIRS "${MBEDTLS_SRC_DIR}/include" CACHE PATH "")
set(MBEDTLS_LIBRARY "mbedtls" CACHE STRING "")
set(MBEDX509_LIBRARY "mbedx509" CACHE STRING "")
set(MBEDCRYPTO_LIBRARY "mbedcrypto" CACHE STRING "")
set(MBEDTLS_LIBRARIES "mbedtls;mbedx509;mbedcrypto" CACHE STRING "")

# Fetch IXWebSocket
FetchContent_Declare(
    ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket.git
    GIT_TAG v11.4.5
)
set(USE_TLS ON CACHE BOOL "Enable TLS support")
set(USE_MBED_TLS ON CACHE BOOL "Use MbedTLS")
set(USE_OPEN_SSL OFF CACHE BOOL "Disable OpenSSL")
set(USE_ZLIB OFF CACHE BOOL "Disable ZLIB compression")
set(IXWEBSOCKET_INSTALL OFF CACHE BOOL "Disable IXWebSocket install")
FetchContent_MakeAvailable(ixwebsocket)

# Application sources
set(SOURCES
    src/main.cpp
    src/App.cpp
    src/OverlayWindow.cpp
    src/WebSocketClient.cpp
    src/Logger.cpp
    src/DeviceInfo.cpp
    src/TimerManager.cpp
)

set(HEADERS
    src/App.h
    src/OverlayWindow.h
    src/WebSocketClient.h
    src/Logger.h
    src/DeviceInfo.h
    src/TimerManager.h
)

# Create executable
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} resources/app.rc)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ixwebsocket
    ws2_32
    gdi32
    user32
    comctl32
    dwmapi
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
