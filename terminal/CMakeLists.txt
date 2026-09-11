add_library(smartshell_terminal INTERFACE)
set_target_properties(smartshell_terminal PROPERTIES EXPORT_NAME terminal)
add_library(SmartShell::terminal ALIAS smartshell_terminal)
add_library(smartshell::terminal ALIAS smartshell_terminal)
target_link_libraries(smartshell_terminal INTERFACE smartshell::core)

install(TARGETS smartshell_terminal EXPORT SmartShellTargets)
