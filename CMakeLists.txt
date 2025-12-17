cmake_minimum_required(VERSION 3.14)
project(mc)
set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILES main.cpp MCEngine.cpp MCEngine.h input.cpp input.hpp gl.c)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Windows with MinGW Installations
if( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND MINGW )
    # if working on Windows but not in the lab
    # update the include directory location
    include_directories("Z:/CSCI441/include")
    # update the lib directory location
    target_link_directories(${PROJECT_NAME} PUBLIC "Z:/CSCI441/lib")
    target_link_libraries(${PROJECT_NAME} opengl32 glfw3 glad gdi32)
# OS X Installations
elseif( APPLE AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    # update the include directory location
    include_directories("/usr/local/include")
    # update the lib directory location
    target_link_directories(${PROJECT_NAME} PUBLIC "/usr/local/lib")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo" glfw3 glad)
# Blanket *nix Installations
elseif( UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    # update the include directory location
    include_directories("/usr/local/include")
    # update the lib directory location
    target_link_directories(${PROJECT_NAME} PUBLIC "/usr/local/lib")
    target_link_libraries(${PROJECT_NAME} GL glfw3 glad)

    # Use -DUSE_CURL=True to enable fetching player skins from online
    # May compile without but requires HTTPS capable curl to work (e.g. libcurl4-openssl-dev on Ubuntu)
    if( USE_CURL )
        target_link_libraries(${PROJECT_NAME} curl)
        add_definitions(-DUSE_CURL)
    endif()
    unset(USE_CURL CACHE)

    # Use -DUSE_OPENAL=True to enable playing sounds
    # Requires OpenAL and ALUT (e.g. libopenal-dev libalut-dev on Ubuntu)
    if( USE_OPENAL )
        target_link_libraries(${PROJECT_NAME} openal alut)
        add_definitions(-DUSE_OPENAL)
    endif()
    unset(USE_OPENAL CACHE)

    # Use -DOPTIMIZE=True to enable compiler optimization
    # May have unexpected results!
    if( OPTIMIZE )
        set(CMAKE_CXX_FLAGS "-O3")
    endif()
    unset(OPTIMIZE CACHE)

endif()