cmake_minimum_required(VERSION 3.16.3)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

project(vulkan_sample)

# build native_app_glue as a static lib
set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
add_library(app-glue STATIC ${APP_GLUE_DIR}/android_native_app_glue.c)

# Build application's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -std=c++2a -DVK_USE_PLATFORM_ANDROID_KHR")

# Force export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# ---- Conan package ----

include("../cmake/conan.cmake")
conan_cmake_run(CONANFILE ./conanfile.txt BASIC_SETUP BUILD missing)

file(GLOB_RECURSE PROJECT_SOURCES "../src/*.cpp")

# ---- Create library ----

add_library(${PROJECT_NAME}
    SHARED
    ${PROJECT_SOURCES}
    ./particle/src/main/cpp/particle.cpp
    ./vulkan_wrapper/vulkan_wrapper.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ../include)
target_include_directories(${PROJECT_NAME} PUBLIC ./vulkan_wrapper)

target_link_libraries(${PROJECT_NAME} PUBLIC ${CONAN_LIBS} app-glue log android)
