cmake_minimum_required(VERSION 3.12)

project(GameBoyAdvanceEmulator  VERSION 1.0.0
        DESCRIPTION "C++ implemented emulator for the Game Boy Advance"
        LANGUAGES CXX)
        
set(DEBUG TRUE)

link_directories(lib)
set(SDL2_DIR "lib/cmake/SDL2")
include_directories(include)
include_directories(include/headers)
include_directories(include/SDL2)

############### Main Build ###############

file(GLOB GBA_SRC CONFIGURE_DEPENDS "src/*.cpp")

add_executable(GBA ${GBA_SRC})

target_sources(GBA PRIVATE "res/icon.rc")

find_package(SDL2 REQUIRED)

target_link_libraries(GBA mingw32)
target_link_libraries(GBA SDL2main)
target_link_libraries(GBA SDL2_ttf)
target_link_libraries(GBA SDL2)

if(DEBUG)
    set(CMAKE_CXX_FLAGS "-Wall -DDEBUG")
else()
    set(CMAKE_CXX_FLAGS "-Wl,-subsystem,windows -O3")  
endif()