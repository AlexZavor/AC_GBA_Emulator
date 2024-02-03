cmake_minimum_required(VERSION 3.12)

project(GameBoyAdvanceEmulator  VERSION 1.0.0
        DESCRIPTION "C implemented emulator for the Game Boy Advance"
        LANGUAGES C)

set(DEBUG TRUE)
        
link_directories(lib)
set(SDL2_DIR "lib/cmake/SDL2")
include_directories(include)

############### Main Build ###############

add_executable(GBA src/main.c)


find_package(SDL2 REQUIRED)

target_link_libraries(GBA mingw32)
target_link_libraries(GBA SDL2main)
target_link_libraries(GBA SDL2)

if(DEBUG)
    set(CMAKE_C_FLAGS "-Wall -DDEBUG")
else()
    set(CMAKE_C_FLAGS "-Wl,-subsystem,windows")  
endif()