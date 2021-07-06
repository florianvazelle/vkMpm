####################################################################################################
# Converts any file into C/C++ source code.
# Example:
# - input file: data.dat
# - output file: data.h
# - variable name declared in output file: DATA
# cmake -DPATH=data.dat -DHEADER=data.h -DGLOBAL=DATA
####################################################################################################

if(EXISTS "${HEADER}")
    if("${HEADER}" IS_NEWER_THAN "${PATH}")
        return()
    endif()
endif()

if(EXISTS "${PATH}")
    file(READ "${PATH}" hex_content HEX)

    string(REPEAT "[0-9a-f]" 32 pattern)
    string(REGEX REPLACE "(${pattern})" "\\1\n" content "${hex_content}")

    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1, " content "${content}")

    string(REGEX REPLACE ", $" "" content "${content}")

    set(array_definition "static const std::vector<unsigned char> ${GLOBAL} =\n{\n${content}\n};")
    
    get_filename_component(file_name ${HEADER} NAME)
    set(source "/**\n * @file ${file_name}\n * @brief Auto generated file.\n */\n#pragma once\n#include <vector>\n${array_definition}\n")

    file(WRITE "${HEADER}" "${source}")
else()
    message("ERROR: ${PATH} doesn't exist")
    return()
endif()