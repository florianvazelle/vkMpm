option(USE_SANITIZER "Enable sanitizers by adding -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined flags" ON)

function(add_sanitizer_options)
    include(CMakeParseArguments)

    cmake_parse_arguments(PARSED_ARGS "" "TARGET" "" ${ARGN})

    if(USE_SANITIZER)
        # Note: if it remains unparsed arguments, here, they can be found in variable PARSED_ARGS_UNPARSED_ARGUMENTS
        if(NOT PARSED_ARGS_TARGET)
            message(FATAL_ERROR "You must provide a target.")
        endif()

        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined>)
        target_link_libraries(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined>)
    endif()
endfunction()
