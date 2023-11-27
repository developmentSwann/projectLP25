cmake_minimum_required(VERSION 3.24)
project(LP25 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(LP25
        configuration.c
        configuration.h
        defines.h
        file-properties.c
        file-properties.h
        files-list.c
        files-list.h
        main.c
        messages.c
        messages.h
        processes.c
        processes.h
        sync.c
        sync.h
        utility.c
        utility.h)
