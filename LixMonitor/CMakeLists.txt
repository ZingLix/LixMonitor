cmake_minimum_required(VERSION 2.8)
project(LixMonitor)
add_definitions(-std=c++14 -g)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../build/)
file(GLOB src "*.cpp")
add_executable(LixMonitor ${src})
target_link_libraries(LixMonitor boost_system boost_regex pthread ssl crypto)
