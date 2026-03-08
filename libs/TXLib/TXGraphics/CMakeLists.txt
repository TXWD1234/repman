# Copyright@TXLib All rights reserved.
# Author: TX Studio: TX_Jerry
# Module: TXGraphics

cmake_minimum_required(VERSION 4.2)

# to defence being included multiple times
if(TARGET TXGraphics)
	return()
endif()

set(TXLib "..")
set(CMakeUtils "${TXLib}/TXCMakeUtils")

include("${CMakeUtils}/optimization.cmake")

# dependencies
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependency/glad")

find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_library(TXGraphics STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include/tx/graphics.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/impl/render_engine.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/impl/rgb.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/render_engine.cpp"
)
tx_add_release_ops(TXGraphics)
target_link_libraries(TXGraphics PUBLIC 
	glfw
	glad
	OpenGL::GL	
	TXMath
)

target_include_directories(TXGraphics PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
)