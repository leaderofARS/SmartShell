include_guard(GLOBAL)

option(SMARTSHELL_ENABLE_CLANG_TIDY "Run clang-tidy during compilation when available" OFF)

if(SMARTSHELL_ENABLE_CLANG_TIDY)
    find_program(SMARTSHELL_CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
    if(SMARTSHELL_CLANG_TIDY_EXECUTABLE)
        set(CMAKE_CXX_CLANG_TIDY "${SMARTSHELL_CLANG_TIDY_EXECUTABLE}" CACHE STRING "" FORCE)
    else()
        message(WARNING "SMARTSHELL_ENABLE_CLANG_TIDY is ON, but clang-tidy was not found.")
    endif()
endif()
