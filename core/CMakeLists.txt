add_library(smartshell_core INTERFACE)
set_target_properties(smartshell_core PROPERTIES EXPORT_NAME core)
add_library(SmartShell::core ALIAS smartshell_core)
add_library(smartshell::core ALIAS smartshell_core)
target_link_libraries(smartshell_core INTERFACE smartshell::options smartshell::warnings)

install(TARGETS smartshell_core EXPORT SmartShellTargets)
