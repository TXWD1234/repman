cmake_minimum_required(VERSION 4.2)

set(libs "${CMAKE_SOURCE_DIR}/libs")
set(src "${CMAKE_SOURCE_DIR}/src")

set(TXLib "${libs}/TXLib")
set(CMakeUtils "${TXLib}/TXCMakeUtils")

include("${CMakeUtils}/optimization.cmake")
include("${CMakeUtils}/cp_dir_to_bin.cmake")

project(TXProject)

# dependencies

add_subdirectory("${TXLib}")

add_executable(TXProject 
	"${src}/main.cpp"
	"${src}/Project.hpp"
)
target_include_directories(TXProject PRIVATE 
	"${CMAKE_SOURCE_DIR}"
	"${libs}"
	"${src}"
)

# project settings
target_compile_features(TXProject PUBLIC cxx_std_20)

# dependencies
target_link_libraries(TXProject PRIVATE
	TXJson
	TXResource
)

# optimization
tx_add_release_ops(TXProject)

# build
set_target_properties(TXProject PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS TXProject
    RUNTIME DESTINATION "./"
)