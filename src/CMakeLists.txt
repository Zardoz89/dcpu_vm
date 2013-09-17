#include(GL)
include(SFML)

set(dcpu-vm_SRCS
    main.cpp
    dcpu.cpp
    disassembler.cpp  
    binasm.cpp
    file.c
    )

set(dcpu-vm_DEVICES_SRCS
    ./devices/gclock.cpp
    ./devices/gkeyboard.cpp
    ./devices/lem1802.cpp
    ./devices/lem1803.cpp
    ./devices/cgm.cpp
    ./devices/speaker.cpp
    ./devices/m35fd.cpp
    )

set(dcpu-vm_SFML_SRCS
    ./sfml/square_gen.cpp
    ./sfml/KeyboardWindow.cpp
    ./sfml/MonitorWindows.cpp
    )

add_executable(dcpu-vm 
    ${dcpu-vm_SRCS}
    ${dcpu-vm_DEVICES_SRCS}
    ${dcpu-vm_SFML_SRCS}
    )

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

