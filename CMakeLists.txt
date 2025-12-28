cmake_minimum_required(VERSION 3.28)
project(OS_Design C)

set(CMAKE_C_STANDARD 11)

add_executable(_S_Design main.c
        memory.h
        memory.c)
