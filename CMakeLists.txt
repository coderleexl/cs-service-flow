cmake_minimum_required(VERSION 3.16)

project(cs-service-flow)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/googletest-release-1.11.0)
add_subdirectory(src)
add_subdirectory(examples)
