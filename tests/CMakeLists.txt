add_test(NAME SmartShell.version COMMAND $<TARGET_FILE:SmartShell> --version)
set_tests_properties(SmartShell.version PROPERTIES PASS_REGULAR_EXPRESSION "SmartShell [0-9]+\\.[0-9]+\\.[0-9]+")
