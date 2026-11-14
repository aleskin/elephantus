# GoogleTest: the system sources if present, otherwise a pinned download.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
if(EXISTS /usr/src/googletest/CMakeLists.txt)
    add_subdirectory(/usr/src/googletest ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
else()
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(googletest)
endif()
