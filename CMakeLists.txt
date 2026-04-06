cmake_minimum_required(VERSION 3.10)
project(dspd C)

set(CMAKE_C_STANDARD 11)

add_executable(dspd main.c
        seat.c
        coach.c
        test.c
        booking.c
        passenger.c
)
