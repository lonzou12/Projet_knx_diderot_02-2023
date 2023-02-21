cmake_minimum_required(VERSION 3.0.0)
project(projet_knx VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# include custom headers and sources
file(GLOB SOURCES 
	./header/*.h
	./header/*.hpp
	./src/*.cpp
)

add_executable(projet_knx main.cpp ${SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
