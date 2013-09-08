project(dcpu-vm)

cmake_minimum_required(VERSION 2.6)
set(PACKAGE_BUGREPORT "luis.panader@gmail.com")
set(PACKAGE_NAME "dcpu-vm")
set(PACKAGE_VERSION "0.0.0a")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME "${PACKAGE_NAME}")

option(BUILD_tests "build the tests" ON)

if(BUILD_tests)
    enable_testing()
endif(BUILD_tests)

set(CMAKE_MODULE_PATH ${dcpu-vm_SOURCE_DIR}/cmake)

set(DCPU-VM_INCLUDE_DIRS
    ${dcpu-vm_SOURCE_DIR}/include
)

include(Platform)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

message( "Procesing Source Code")
add_subdirectory(src)

message( "Procesing Assets")
add_subdirectory(assets)


if(BUILD_tests)
    # Some day....
endif(BUILD_tests)

