cmake_minimum_required(VERSION 3.25)

project(SmartShell
    VERSION 0.1.0
    DESCRIPTION "Native Windows terminal and shell platform"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(SMARTSHELL_BUILD_TESTING "Build SmartShell tests" ON)
option(SMARTSHELL_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(SMARTSHELL_ENABLE_ASAN "Enable AddressSanitizer where supported" OFF)
option(SMARTSHELL_ENABLE_LTO "Enable link-time optimization for release builds" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ProjectOptions)
include(CompilerWarnings)
include(StaticAnalysis)

smartshell_setup_project_options()

add_library(smartshell::options ALIAS smartshell_options)
add_library(smartshell::warnings ALIAS smartshell_warnings)

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(terminal)
add_subdirectory(shell)
add_subdirectory(session)
add_subdirectory(ui)
add_subdirectory(app)

if(SMARTSHELL_BUILD_TESTING)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()

install(EXPORT SmartShellTargets
    FILE SmartShellTargets.cmake
    NAMESPACE SmartShell::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SmartShell"
)
install(TARGETS smartshell_options smartshell_warnings EXPORT SmartShellTargets)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SmartShellConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/SmartShellConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SmartShell"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/SmartShellConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/SmartShellConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/SmartShellConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/SmartShell"
)
