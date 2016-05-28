# CMake entry point
cmake_minimum_required (VERSION 2.6)
project (OceanCurrents)

find_package(OpenGL REQUIRED)


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

add_subdirectory (external)

cmake_policy(SET CMP0026 OLD)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

include_directories(
	external/AntTweakBar-1.16/include/
	external/glfw-3.1.2/include/GLFW/
	external/glm-0.9.7.1/
	external/glew-1.13.0/include/
	external/assimp-3.0.1270/include/
	external/netCDF/include/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW_1130
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

#OceanCurrents
add_executable(OceanCurrents
	OceanCurrents/main.cpp
	OceanCurrents/ApplicationContext.hpp
	OceanCurrents/ApplicationContext.cpp
	OceanCurrents/controller.hpp
	OceanCurrents/controller.cpp
	OceanCurrents/GeoArray.h
	OceanCurrents/GeoVolume.h
	OceanCurrents/GeoVolume.cpp
	OceanCurrents/NetCDFArray.cpp
	OceanCurrents/NetCDFArray.h
	OceanCurrents/olic.hpp
	OceanCurrents/olic.cpp
	OceanCurrents/utils.h
	OceanCurrents/vectorField.hpp


	utils/objectLoader.cpp
	utils/objectLoader.hpp
	utils/imageLoader.cpp
	utils/imageLoader.hpp
	utils/shaderProgram.cpp
	utils/shaderProgram.hpp

	OceanCurrents/OceanCurrents.frag
	OceanCurrents/OceanCurrents.vert
)

target_link_libraries(OceanCurrents
	${ALL_LIBS}
	assimp
	netCDF
)
# Xcode and Visual working directories
set_target_properties(OceanCurrents PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/OceanCurrents/")
create_target_launcher(OceanCurrents WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/OceanCurrents/")

SOURCE_GROUP(utils REGULAR_EXPRESSION ".*/utils/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*[frag|vert]$" )
