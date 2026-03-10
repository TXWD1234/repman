cmake_minimum_required(VERSION 4.2)

set(libs "${CMAKE_SOURCE_DIR}/libs")
set(src "${CMAKE_SOURCE_DIR}/src")

set(TXLib "${libs}/TXLib")
set(CMakeUtils "${TXLib}/TXCMakeUtils")

include("${CMakeUtils}/optimization.cmake")
include("${CMakeUtils}/cp_dir_to_bin.cmake")

project(repman)

# install
# if(NOT DEFINED CMAKE_INSTALL_PREFIX) # ChatGPT
#     set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install prefix")
# endif()
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT) # Gemini
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install prefix to ~/.local" FORCE)
endif()

# dependencies

add_subdirectory("${TXLib}")

add_executable(repman 
	"${src}/main.cpp"
	"${src}/Project.hpp"
)
target_include_directories(repman PRIVATE 
	"${CMAKE_SOURCE_DIR}"
	"${libs}"
	"${src}"
)

# project settings
target_compile_features(repman PUBLIC cxx_std_20)

# dependencies
target_link_libraries(repman PRIVATE
	TXJson
	TXResource
)

# optimization
tx_add_release_ops(repman)

# build
set_target_properties(repman PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS repman
    RUNTIME DESTINATION bin
)