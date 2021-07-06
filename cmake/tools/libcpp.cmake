option(USE_LIBCPP "Use libc++ by adding -stdlib=libc++ flag if availbale." ON)

if(NOT WIN32 AND USE_LIBCPP)
    include(CheckCXXSourceCompiles)
    include(CMakePushCheckState)

    cmake_push_check_state()

    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -stdlib=libc++")

    check_cxx_source_compiles("
        #include<type_traits>
        int main() { return std::is_same_v<int, char>; }
    " HAS_LIBCPP)

    if(NOT HAS_LIBCPP)
        message(VERBOSE "The option USE_LIBCPP is set (by default) but libc++ is not available. The flag will not be added to the target.")
    endif()

    cmake_pop_check_state()
endif()

function(add_libcpp_option)
    include(CMakeParseArguments)

    cmake_parse_arguments(PARSED_ARGS "" "TARGET" "" ${ARGN})

    if(HAS_LIBCPP)
        # Note: if it remains unparsed arguments, here, they can be found in variable PARSED_ARGS_UNPARSED_ARGUMENTS
        if(NOT PARSED_ARGS_TARGET)
            message(FATAL_ERROR "You must provide a target.")
        endif()

        target_compile_options(${PARSED_ARGS_TARGET} BEFORE PRIVATE -stdlib=libc++)
    endif()
endfunction()
