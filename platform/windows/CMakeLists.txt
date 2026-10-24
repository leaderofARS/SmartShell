if(NOT WIN32)
    message(FATAL_ERROR "SmartShell is currently supported only on Windows.")
endif()

add_library(smartshell_platform_windows STATIC
    filesystem/filesystem.cpp
    environment/environment.cpp
    process/process.cpp
    threading/thread.cpp
    info/os_info.cpp
)

set_target_properties(smartshell_platform_windows PROPERTIES EXPORT_NAME platform_windows)
add_library(SmartShell::platform_windows ALIAS smartshell_platform_windows)
add_library(smartshell::platform_windows ALIAS smartshell_platform_windows)

target_include_directories(smartshell_platform_windows PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(smartshell_platform_windows
    PUBLIC  smartshell::core
    PRIVATE kernel32 user32 advapi32 shell32 shlwapi ole32
)

target_compile_definitions(smartshell_platform_windows PUBLIC
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00
)

install(TARGETS smartshell_platform_windows EXPORT SmartShellTargets)

