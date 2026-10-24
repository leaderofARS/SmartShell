add_library(smartshell_core STATIC
    # Errors
    errors/error.cpp
    # Text
    text/encoding.cpp
    # Logging
    logging/logger.cpp
)

set_target_properties(smartshell_core PROPERTIES EXPORT_NAME core)
add_library(SmartShell::core ALIAS smartshell_core)
add_library(smartshell::core ALIAS smartshell_core)

target_include_directories(smartshell_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(smartshell_core
    PUBLIC  smartshell::options smartshell::warnings
)

install(TARGETS smartshell_core EXPORT SmartShellTargets)
