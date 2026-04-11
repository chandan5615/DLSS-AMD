cmake_minimum_required(VERSION 3.20)
project(AFGlobalFrameGenerator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable manifest generation to avoid mt.exe issues
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")

# Configuration for x64 Windows build
set(CMAKE_GENERATOR_PLATFORM x64)

# Build options
option(ENABLE_MINHOOK "Enable MinHook dependency (requires manual installation or vcpkg)" OFF)

# Add FetchContent for dependency management
include(FetchContent)

# Fetch nlohmann/json (lightweight, always fetch this)
message(STATUS "Fetching nlohmann/json library...")

# Create a fake include structure for nlohmann/json single header
set(NLOHMANN_JSON_INCLUDE_DIR "${CMAKE_BINARY_DIR}/_deps/nlohmann-include")

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
    DOWNLOAD_NO_EXTRACT TRUE
    DOWNLOAD_DIR "${NLOHMANN_JSON_INCLUDE_DIR}/nlohmann"
    DOWNLOAD_NAME "json.hpp"
)

FetchContent_MakeAvailable(nlohmann_json)

# Create interface library for nlohmann/json
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE "${NLOHMANN_JSON_INCLUDE_DIR}")

message(STATUS "nlohmann/json library available at: ${NLOHMANN_JSON_INCLUDE_DIR}")

# MinHook handling
set(MINHOOK_FOUND FALSE)

if(ENABLE_MINHOOK)
    message(STATUS "MinHook support enabled. Searching for MinHook library...")
    
    # Try to find MinHook in system or vcpkg
    find_library(MINHOOK_LIB NAMES minhook MinHook QUIET)
    find_path(MINHOOK_INCLUDE NAMES MinHook.h QUIET)
    
    if(MINHOOK_LIB AND MINHOOK_INCLUDE)
        set(MINHOOK_FOUND TRUE)
        message(STATUS "Found MinHook: ${MINHOOK_LIB}")
    else()
        message(WARNING "MinHook not found. DLL compilation will be skipped. Install via: vcpkg install minhook:x64-windows")
    endif()
else()
    message(STATUS "MinHook support disabled. DLL will not be built. Use: cmake -DENABLE_MINHOOK=ON to enable")
endif()

# Find required packages
find_package(directx-headers QUIET)
find_package(d3d12 QUIET)

# Common source files
set(COMMON_SOURCES
    src/common/config.cpp
    src/common/logger.cpp
)

set(COMMON_HEADERS
    src/common/config.h
    src/common/constants.h
    src/common/logger.h
)

# Executable source files
set(EXE_SOURCES
    src/afglobal_exe/main.cpp
    src/afglobal_exe/tray_app.cpp
    src/afglobal_exe/process_monitor.cpp
    src/afglobal_exe/injector.cpp
    src/afglobal_exe/named_pipe_client.cpp
    src/afglobal_exe/gui_window.cpp
    src/afglobal_exe/gui_log_viewer.cpp
    src/afglobal_exe/gui_process_manager.cpp
    ${COMMON_SOURCES}
)

set(EXE_HEADERS
    src/afglobal_exe/tray_app.h
    src/afglobal_exe/process_monitor.h
    src/afglobal_exe/injector.h
    src/afglobal_exe/named_pipe_client.h
    src/afglobal_exe/gui_window.h
    src/afglobal_exe/gui_log_viewer.h
    src/afglobal_exe/gui_process_manager.h
    ${COMMON_HEADERS}
)

# DLL source files
set(DLL_SOURCES
    src/afglobal_dll/dllmain.cpp
    src/afglobal_dll/dxgi_hooks.cpp
    src/afglobal_dll/frame_interpolator.cpp
    src/afglobal_dll/amf_optical_flow.cpp
    src/afglobal_dll/named_pipe_server.cpp
    ${COMMON_SOURCES}
)

set(DLL_HEADERS
    src/afglobal_dll/dxgi_hooks.h
    src/afglobal_dll/frame_interpolator.h
    src/afglobal_dll/amf_optical_flow.h
    src/afglobal_dll/named_pipe_server.h
    ${COMMON_HEADERS}
)

# Create executable (always built)
add_executable(afglobal_exe WIN32 ${EXE_SOURCES} ${EXE_HEADERS})

# Create DLL only if MinHook is available
if(MINHOOK_FOUND)
    message(STATUS "Building DLL with MinHook support")
    add_library(afglobal_hook SHARED ${DLL_SOURCES} ${DLL_HEADERS})
    
    set_target_properties(afglobal_hook PROPERTIES
        OUTPUT_NAME "afglobal_hook"
        PREFIX ""
    )
    
    # Include directories for DLL
    target_include_directories(afglobal_hook PRIVATE
        src/common
        src/afglobal_dll
        ${MINHOOK_INCLUDE}
    )
    
    # Link libraries for DLL
    target_link_libraries(afglobal_hook PRIVATE
        nlohmann_json
        ${MINHOOK_LIB}
        dxgi.lib
        d3d12.lib
        user32.lib
        kernel32.lib
        psapi.lib
        shell32.lib
    )
    
    # Compiler settings for DLL
    if(MSVC)
        target_compile_definitions(afglobal_hook PRIVATE _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
        set_target_properties(afglobal_hook PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        target_compile_options(afglobal_hook PRIVATE /O2 /Oi)
    endif()
else()
    message(STATUS "MinHook not available - DLL will not be built")
    message(STATUS "To build DLL: vcpkg install minhook:x64-windows && cmake -DENABLE_MINHOOK=ON ..")
endif()

# Set executable output name
set_target_properties(afglobal_exe PROPERTIES
    OUTPUT_NAME "afglobal"
    VS_DEBUGGER_ENVIRONMENT "PATH=$(DebuggerFoldersPath)%(PATH)"
)

# Include directories for executable
target_include_directories(afglobal_exe PRIVATE
    src/common
    src/afglobal_exe
)

# Add Windows SDK library directories
link_directories("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/um/x64")

# Link libraries for executable
target_link_libraries(afglobal_exe PRIVATE
    nlohmann_json
    shell32.lib
    user32.lib
    gdi32.lib
    psapi.lib
    advapi32.lib
    kernel32.lib
    winmm.lib
)

# Set compiler flags
if(MSVC)
    # Disable deprecation warnings for Windows API
    target_compile_definitions(afglobal_exe PRIVATE _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
    
    if(MINHOOK_FOUND)
        target_compile_definitions(afglobal_hook PRIVATE _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
    endif()
    
    # Set runtime library
    set_target_properties(afglobal_exe PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    if(MINHOOK_FOUND)
        set_target_properties(afglobal_hook PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
    
    # Enable optimizations
    target_compile_options(afglobal_exe PRIVATE /O2 /Oi)
    
    if(MINHOOK_FOUND)
        target_compile_options(afglobal_hook PRIVATE /O2 /Oi)
    endif()
endif()

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Install targets
if(MINHOOK_FOUND)
    install(TARGETS afglobal_exe afglobal_hook
        RUNTIME DESTINATION "Program Files/AMDGlobalFG"
        LIBRARY DESTINATION "Program Files/AMDGlobalFG"
    )
else()
    install(TARGETS afglobal_exe
        RUNTIME DESTINATION "Program Files/AMDGlobalFG"
    )
endif()

# Install config file
install(FILES afglobal_config.json
    DESTINATION "Program Files/AMDGlobalFG"
)
