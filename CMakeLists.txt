cmake_minimum_required(VERSION 3.25)

include(${CMAKE_CURRENT_SOURCE_DIR}/../gfxengine/cmake/gfxengine.cmake)

project(blocks VERSION 0.1 LANGUAGES CXX)

set(PROJECT_SOURCES
	src/application.cpp
	src/block.cpp
	src/block.hpp
	src/chunk.cpp
	src/chunk.hpp
	src/world.cpp
	src/world.hpp
)

gfxengine_application(blocks ${CMAKE_CURRENT_SOURCE_DIR}/../gfxengine "${PROJECT_SOURCES}")
