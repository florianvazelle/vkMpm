#
# vkMpm
#

cmake_minimum_required(VERSION 3.16.3)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(cmake/tools/guards.cmake)
include(cmake/tools/warnings.cmake)

#
# Project configuration
#

project(
    vkMpm
    DESCRIPTION "A Vulkan application to simulate 2D material with the Material Point Method"
    HOMEPAGE_URL "https://github.com/florianvazelle/vkMpm"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

include(cmake/tools/libcpp.cmake)
include(cmake/tools/sanitizer.cmake)
include(cmake/tools/compile-shader.cmake)

#
# Define executable target
#

file(GLOB_RECURSE PROJECT_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE PROJECT_HEADERS "${CMAKE_SOURCE_DIR}/include/*.hpp")

include(cmake/external/poike.cmake)

# ---- Conan package ----

include("${CMAKE_SOURCE_DIR}/cmake/conan.cmake")

conan_check(REQUIRED)
conan_cmake_run(
    CONANFILE
    conanfile.txt
    BASIC_SETUP
    CONAN_COMMAND
    ${CONAN_CMD}
    CMAKE_TARGETS
    BUILD
    missing)
include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()


# ---- Create executable ----

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS} "${CMAKE_SOURCE_DIR}/app/particle.cpp")

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${florianvazelle_poike_SOURCE_DIR}/include)

if(CONAN_TARGETS)
    target_link_libraries(${PROJECT_NAME} ${CONAN_TARGETS} poike)
else()
    target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS} poike)
    foreach(_LIB ${CONAN_LIBS_RELEASE})
        target_link_libraries(${PROJECT_NAME} optimized ${_LIB})
    endforeach()
    foreach(_LIB ${CONAN_LIBS_DEBUG})
        target_link_libraries(${PROJECT_NAME} debug ${_LIB})
    endforeach()
endif()

target_set_warnings(
    ${PROJECT_NAME}
    ENABLE ALL
    AS_ERROR ALL
    DISABLE Annoying
)

# ---- Compile shader into SPIR-V ----

file(GLOB_RECURSE SHADERS "${CMAKE_SOURCE_DIR}/assets/shaders/*.vert" "${CMAKE_SOURCE_DIR}/assets/shaders/*.frag" "${CMAKE_SOURCE_DIR}/assets/shaders/*.comp")

compile_shaders(TARGETS ${SHADERS})