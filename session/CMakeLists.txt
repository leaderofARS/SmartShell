add_library(smartshell_session INTERFACE)
set_target_properties(smartshell_session PROPERTIES EXPORT_NAME session)
add_library(SmartShell::session ALIAS smartshell_session)
add_library(smartshell::session ALIAS smartshell_session)
target_link_libraries(smartshell_session INTERFACE
    smartshell::shell
    smartshell::terminal
    smartshell::platform_windows
)

install(TARGETS smartshell_session EXPORT SmartShellTargets)
