project(dcpu-vm)

cmake_minimum_required(VERSION 2.6)
set(PACKAGE_BUGREPORT "luis.panader@gmail.com")
set(PACKAGE_NAME "dcpu-vm")
set(PACKAGE_VERSION "0.1")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME "${PACKAGE_NAME}")

option(BUILD_tests "build the tests" ON)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message( "Release mode")
    set (LOG_ENABLE 0)
else (CMAKE_BUILD_TYPE STREQUAL "Release")
    set (LOG_ENABLE 1)
endif (CMAKE_BUILD_TYPE STREQUAL "Release")

if(BUILD_tests)
    enable_testing()
endif(BUILD_tests)

set(CMAKE_MODULE_PATH ${dcpu-vm_SOURCE_DIR}/cmake)

set(DCPU-VM_INCLUDE_DIRS
    ${PROJECT_BINARY_DIR}
    ${dcpu-vm_SOURCE_DIR}/include
)

include(Platform)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

message( "Procesing Source Code")
add_subdirectory(include)
add_subdirectory(src)

message( "Procesing Assets")
add_subdirectory(assets)


if(BUILD_tests)
    # Some day....
endif(BUILD_tests)

