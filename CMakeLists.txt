cmake_minimum_required(VERSION 4.2)
project(dspd C)

set(CMAKE_C_STANDARD 11)

add_executable(dspd main.c
        seat.c
        coach.h
        train.h
        coach.c
        test.h
        test.c
        struct.h
        booking.c
        passenger.h
        passenger.c
)
