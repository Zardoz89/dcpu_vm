if (CMAKE_HOST_APPLE)
    # OS X is a Unix, but it's not a normal Unix as far as search paths go.
    set(DCPU_VM_SEARCH_PATHS
        /Applications/Xcode.app/Contents/Developer/Platforms.MacOSX.platform/Developer/SDKs
        ~/Library/Frameworks
        /Library/Frameworks
    )
endif (CMAKE_HOST_APPLE)
set(DCPU_VM_SEARCH_PATHS
    ${SFML_ROOT}
    $ENV{SFML_ROOT}
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt
    ${DCPU_VM_SEARCH_PATHS}
)


if (MSVC)
    set(PLATFORM_FLAGS)
    set(DCPU_VM_SEARCH_PATHS
        "C:/Program Files (x86)/Microsoft Visual Studio 11.0/VC"
    )
else (MSVC)
    set(PLATFORM_FLAGS "-Wall" ) # Generic flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
    if (CMAKE_HOST_WIN32)
      set(DCPU_VM_SEARCH_PATHS
          C:/MinGW
          )

    endif (CMAKE_HOST_WIN32)

    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
endif (MSVC)

add_definitions(${PLATFORM_FLAGS})



