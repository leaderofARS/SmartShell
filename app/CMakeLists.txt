add_executable(SmartShell main.cpp)
set_target_properties(SmartShell PROPERTIES EXPORT_NAME app)
add_executable(SmartShell::app ALIAS SmartShell)
target_link_libraries(SmartShell PRIVATE
    smartshell::options
    smartshell::warnings
    smartshell::ui
    smartshell::session
)
target_compile_definitions(SmartShell PRIVATE
    SMARTSHELL_VERSION="${PROJECT_VERSION}"
)

# MinGW does not infer the wide-character console entry point from wmain.
if(MINGW)
    target_link_options(SmartShell PRIVATE -municode)
endif()

set_target_properties(SmartShell PROPERTIES
    OUTPUT_NAME "SmartShell"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
)

install(TARGETS SmartShell
    EXPORT SmartShellTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
