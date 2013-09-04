if (CMAKE_HOST_APPLE)
    set(TRILLEK_SEARCH_PATHS
        /usr
        /Applications/Xcode.app/Contents/Developer/Platforms.MacOSX.platform/Developer/SDKs
        /Library/Frameworks
        /usr/local
        /opt/local
    )
else (CMAKE_HOST_APPLE)
    # OS X is a Unix, but it's not a normal Unix as far as search paths go.
    if (CMAKE_HOST_UNIX)
        set(TRILLEK_SEARCH_PATHS
            /usr
            /usr/local
            /opt/local
        )
    endif (CMAKE_HOST_UNIX)
endif (CMAKE_HOST_APPLE)

if (CMAKE_HOST_WIN32)
    message(WARNING "This build has only been tested on MinGW 4.7")
    set(TRILLEK_SEARCH_PATHS
        C:/MinGW
    )
endif (CMAKE_HOST_WIN32)

if (MSVC)
    message(WARNING "This build has not yet been tested with VC++")
    set(PLATFORM_FLAGS)
else (MSVC)
    set(PLATFORM_FLAGS "-Wall" "-Wno-packed-bitfield-compat" "-std=c++11" )
    
    set(CMAKE_CSS_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif (MSVC)

add_definitions(${PLATFORM_FLAGS})



