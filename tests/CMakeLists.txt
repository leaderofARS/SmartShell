set(UNIT_TESTS
    test_types
    test_errors
    test_text
    test_logging
    test_time
    test_config
    test_handles
    test_filesystem
    test_environment
    test_process
)

foreach(test_name ${UNIT_TESTS})
    add_executable(${test_name} unit/${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE
        smartshell::options
        smartshell::warnings
        smartshell::core
        smartshell::platform_windows
    )
    add_test(NAME SmartShell.${test_name} COMMAND ${test_name})
endforeach()

add_test(NAME SmartShell.version COMMAND $<TARGET_FILE:SmartShell> --version)
set_tests_properties(SmartShell.version PROPERTIES PASS_REGULAR_EXPRESSION "SmartShell [0-9]+\\.[0-9]+\\.[0-9]+")

