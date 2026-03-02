cmake_minimum_required(VERSION 3.16.3)

project(matchable_test_buildtimes)

option(OMIT_BY_INDEX "Omit by index" OFF)

if(OMIT_BY_INDEX)
    add_definitions(-DMATCHABLE_OMIT_BY_INDEX)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra")

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(matchable_test_buildtimes ${SRC_FILES})
