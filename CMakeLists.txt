cmake_minimum_required(VERSION 3.10)
project(HostFxrApiTest)

set(CMAKE_CXX_STANDARD 17)

add_executable(HostFxrApiTest main.cpp)

target_include_directories(HostFxrApiTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ldl -fPIC")