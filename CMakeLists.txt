cmake_minimum_required(VERSION 3.10)
project(Codigo C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(Codigo
        job_control.c
        job_control.h
        Shell_project.c)
