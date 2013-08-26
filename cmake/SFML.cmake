
message(STATUS "Looking for sfml headers")
find_path(SFML_INCLUDE_DIR
    NAMES SFML/Window.hpp
    HINTS
    PATH_SUFFIXES include Headers
    PATHS ${TRILLEK_SEARCH_PATHS}
)

message(STATUS "Looking for sfml-system library")
find_library(SFML_SYSTEM_LIBRARY
    NAMES sfml-system
    HINTS
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_PATH
    CMAKE_FIND_FRAMEWORK NEVER
    PATH_SUFFIXES lib lib64
    PATHS ${TRILLEK_SEARCH_PATHS}
)

message(STATUS "Looking for sfml-window library")
find_library(SFML_WINDOW_LIBRARY
    NAMES sfml-window
    HINTS
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_PATH
    CMAKE_FIND_FRAMEWORK NEVER
    PATH_SUFFIXES lib lib64
    PATHS ${TRILLEK_SEARCH_PATHS}
)

if (SFML_SYSTEM_LIBRARY)
    if (SFML_WINDOW_LIBRARY)
        set (SFML_LIBRARIES "${SFML_SYSTEM_LIBRARY}" "${SFML_WINDOW_LIBRARY}")
    else(SFML_WINDOW_LIBRARY)
        message(FATAL_ERROR "sfml-window library not found")
    endif(SFML_WINDOW_LIBRARY)
else(SFML_SYSTEM_LIBRARY)
    message(FATAL_ERROR "sfml-system library not found")
endif(SFML_SYSTEM_LIBRARY)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SFML SFML_LIBRARIES SFML_INCLUDE_DIR)

