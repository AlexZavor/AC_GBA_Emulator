cmake_minimum_required(VERSION 3.12)

project(GameBoyAdvanceEmulator  VERSION 1.0.0
        DESCRIPTION "C implemented emulator for the Game Boy Advance"
        LANGUAGES C)
        
# Debug system that can be set TRUE or FALSE
set(DEBUG TRUE)

#SDL Include Directories
set(SDL2_DIR "lib/cmake/SDL2")
include_directories(include/headers)
include_directories(include/SDL2)

############### Main Build ###############

file(GLOB_RECURSE GBA_SRC CONFIGURE_DEPENDS "src/***.c")

add_executable(GBA ${GBA_SRC})

target_sources(GBA PRIVATE "res/icon.rc")

find_package(SDL2 REQUIRED)

if(WIN32)
    link_directories(lib)
    target_link_libraries(GBA mingw32)
    target_link_libraries(GBA SDL2main)
    target_link_libraries(GBA SDL2_ttf)
    target_link_libraries(GBA SDL2::SDL2)
else()
    target_link_libraries(GBA SDL2main)
    target_link_libraries(GBA SDL2_ttf)
    target_link_libraries(GBA SDL2)
endif()

if(DEBUG)
    if(WIN32)
    set(CMAKE_C_FLAGS "-Wl,--subsystem,console -O3 -DDEBUG")
    else()
    set(CMAKE_C_FLAGS "-Wall -DDEBUG")
    endif()
else()
    if(WIN32)
    set(CMAKE_C_FLAGS "-Wl,--subsystem,windows -O3")  
    else()
    set(CMAKE_C_FLAGS "-Wl, -O3")  
    endif()
endif()