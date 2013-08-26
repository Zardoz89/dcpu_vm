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
    ${dcpu-vm_SOURCE_DIR}/src
)

include(Platform)

add_subdirectory(src)

if(BUILD_tests)
endif(BUILD_tests)

