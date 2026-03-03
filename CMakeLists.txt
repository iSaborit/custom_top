cmake_minimum_required(VERSION 4.1)
project(custom_top C)

set(CMAKE_C_STANDARD 17)

find_package(Curses REQUIRED)

add_executable(custom_top src/main.c
        src/process.c
        include/process.h
        src/state.c
        include/state.h
        include/ui.h
        src/ui.c
        include/utils.h
        src/utils.c
        include/sysinfo.h
        src/sysinfo.c
        src/app_state.c)

target_include_directories(custom_top PRIVATE ${CURSES_INCLUDE_DIR})
target_link_libraries(custom_top ${CURSES_LIBRARIES})
