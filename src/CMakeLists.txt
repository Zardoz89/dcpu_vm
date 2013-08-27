#include(GL)
include(SFML)

FIND_PACKAGE(Threads)

set(dcpu-vm_SRCS
    main.cpp
    dcpu.cpp
    disassembler.cpp  
    fake_lem1802.cpp
    lem1802.cpp
    lem1803.cpp
    )

add_executable(dcpu-vm ${dcpu-vm_SRCS})

include_directories(dcpu-vm
    ${DCPU-VM_INCLUDE_DIRS}
#    ${OPENGL_INCLUDE_DIR}
    ${SFML_INCLUDE_DIR}
)

target_link_libraries(dcpu-vm
    ${CMAKE_THREAD_LIBS_INIT}
#    ${OPENGL_LIBRARIES}
    ${SFML_LIBRARIES}
)

INSTALL(TARGETS dcpu-vm DESTINATION ${dcpu-vm_SOURCE_DIR})

