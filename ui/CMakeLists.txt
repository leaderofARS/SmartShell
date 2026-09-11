add_library(smartshell_ui INTERFACE)
set_target_properties(smartshell_ui PROPERTIES EXPORT_NAME ui)
add_library(SmartShell::ui ALIAS smartshell_ui)
add_library(smartshell::ui ALIAS smartshell_ui)
target_link_libraries(smartshell_ui INTERFACE
    smartshell::terminal
    smartshell::platform_windows
)

install(TARGETS smartshell_ui EXPORT SmartShellTargets)
