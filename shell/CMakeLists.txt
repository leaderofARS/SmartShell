add_library(smartshell_shell INTERFACE)
set_target_properties(smartshell_shell PROPERTIES EXPORT_NAME shell)
add_library(SmartShell::shell ALIAS smartshell_shell)
add_library(smartshell::shell ALIAS smartshell_shell)
target_link_libraries(smartshell_shell INTERFACE smartshell::core smartshell::platform_windows)

install(TARGETS smartshell_shell EXPORT SmartShellTargets)
