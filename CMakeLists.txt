cmake_minimum_required(VERSION 2.5)
project(portmapping)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread")

add_executable(enc enc.cpp encrypto.cpp Verification.cpp)
add_executable(dec dec.cpp encrypto.cpp)

target_link_libraries(enc
    pthread
    )

target_link_libraries(dec
        pthread
        )
