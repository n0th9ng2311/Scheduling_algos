cmake_minimum_required(VERSION 4.1)
project(wor)

set(CMAKE_CXX_STANDARD 20)

add_executable(wor main.cpp
        CommonUtils.h
        SJF.h
        FCFS.h
        Priority.h
        SRJF.h
        RR.h
        choice.h)
