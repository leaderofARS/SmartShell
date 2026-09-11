include_guard(GLOBAL)

function(smartshell_setup_project_options)
    add_library(smartshell_options INTERFACE)
    set_target_properties(smartshell_options PROPERTIES EXPORT_NAME options)
    target_compile_features(smartshell_options INTERFACE cxx_std_23)

    set(CMAKE_CXX_EXTENSIONS OFF PARENT_SCOPE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON PARENT_SCOPE)

    if(MSVC)
        target_compile_options(smartshell_options INTERFACE
            /permissive-
            /Zc:__cplusplus
            /Zc:preprocessor
            /utf-8
        )
    endif()

    if(SMARTSHELL_ENABLE_LTO)
        include(CheckIPOSupported)
        check_ipo_supported(RESULT smartshell_ipo_supported OUTPUT smartshell_ipo_error)
        if(smartshell_ipo_supported)
            set_property(TARGET smartshell_options PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        else()
            message(WARNING "LTO requested but unavailable: ${smartshell_ipo_error}")
        endif()
    endif()

    if(SMARTSHELL_ENABLE_ASAN)
        if(MSVC)
            target_compile_options(smartshell_options INTERFACE /fsanitize=address)
            target_link_options(smartshell_options INTERFACE /fsanitize=address)
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(smartshell_options INTERFACE -fsanitize=address -fno-omit-frame-pointer)
            target_link_options(smartshell_options INTERFACE -fsanitize=address)
        else()
            message(WARNING "AddressSanitizer is not configured for ${CMAKE_CXX_COMPILER_ID}.")
        endif()
    endif()
endfunction()
