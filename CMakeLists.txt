cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0026 OLD)
project (CS380)

find_package(OpenGL REQUIRED)

if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

# Compile external dependencies
add_subdirectory (ExternalProjects)

# On Visual 2005 and above, this module can set the debug working directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/ExternalProjects/cmake-modules-master")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

if(INCLUDE_DISTRIB)
	add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)

include_directories(
  ExternalProjects/glfw-3.0.3/include/GLFW/
  ExternalProjects/glm-0.9.4.0/
  ExternalProjects/glew-1.9.0/include/
  .
)

set(ALL_LIBS
  ${OPENGL_LIBRARY}
  GLFW_303
  GLEW_190
)

add_definitions(
  -DTW_STATIC
  -DTW_NO_LIB_PRAGMA
  -DTW_NO_DIRECT3D
  -DGLEW_STATIC
  -D_CRT_SECURE_NO_WARNINGS
)

# Skeleton Project
add_executable(SkeletonProject
	Skeleton/main.cpp
	common/shader.cpp
	common/shader.hpp
	Skeleton/TransformVertexShader.glsl
	Skeleton/ColorFragmentShader.glsl)

target_link_libraries(SkeletonProject ${ALL_LIBS})

# Xcode and Visual Studio working directories
set_target_properties(SkeletonProject PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Skeleton/")
create_target_launcher(SkeletonProject WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Skeleton/")

# Lab session 1
add_executable(Lab1
	Lab1/main.cpp
	common/shader.cpp
	common/shader.hpp
	Lab1/VertexShader.glsl
	Lab1/FragmentShader.glsl)

target_link_libraries(Lab1 ${ALL_LIBS})

# Xcode and Visual Studio working directories
set_target_properties (Lab1 PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Lab1/")
create_target_launcher(Lab1 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Lab1/")

# Lab session 2
add_executable(Lab2
	Lab2/main.cpp
	common/shader.cpp
	common/shader.hpp
	common/model.cpp
	common/model.hpp
	Lab2/VertexShader.glsl
	Lab2/FragmentShader.glsl)

target_link_libraries(Lab2 ${ALL_LIBS})

# Xcode and Visual Studio working directories
set_target_properties(Lab2 PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Lab2/")
create_target_launcher(Lab2 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Lab2/")

# Homework 2: Object manipulation with arcball
add_executable(Homework2
	Homework2/main.cpp
	common/shader.cpp
	common/shader.hpp
	common/model.cpp
	common/model.hpp
	common/geometry.hpp
	common/affine.hpp
	common/arcball.hpp
	Homework2/VertexShader.glsl
	Homework2/FragmentShader.glsl)

target_link_libraries(Homework2 ${ALL_LIBS})

# Xcode and Visual Studio working directories
set_target_properties(Homework2 PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Homework2/")
create_target_launcher(Homework2 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Homework2/")

# Homework 3: 3x3x1 rubik cube simulation
add_executable(Homework3
	Homework3/main.cpp
	common/shader.cpp
	common/shader.hpp
	common/model.cpp
	common/model.hpp
	common/geometry.hpp
	common/affine.hpp
	common/arcball.hpp # arcball helper function
	common/picking.hpp # picking helper function
	Homework3/VertexShader.glsl
	Homework3/FragmentShader.glsl
	Homework3/PickingVertexShader.glsl
	Homework3/PickingFragmentShader.glsl)

target_link_libraries(Homework3 ${ALL_LIBS})

# Xcode and Visual Studio working directories
set_target_properties(Homework3 PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Homework3/")
create_target_launcher(Homework3 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Homework3/")

# Lab session 3
add_executable(Lab3
	Lab3/main.cpp
	Lab3/VertexShader.glsl
	Lab3/FragmentShader.glsl
	common/shader.cpp
	common/shader.hpp
	common/model.cpp
	common/model.hpp
	common/geometry.hpp
	common/affine.hpp
	common/arcball.hpp)

target_link_libraries(Lab3 ${ALL_LIBS})

# Xcode and Visual Studio working directories
set_target_properties(Lab3 PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Lab3/")
create_target_launcher(Lab3 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Lab3/")

# Lab session 4
add_executable(Lab4
	Lab4/main.cpp
	Lab4/VertexShader.glsl
	Lab4/FragmentShader.glsl
	Lab4/BumpVertexShader.glsl
	Lab4/BumpFragmentShader.glsl
	Lab4/EnvVertexShader.glsl
	Lab4/EnvFragmentShader.glsl
	common/shader.cpp
	common/shader.hpp
	common/model.cpp
	common/model.hpp
	common/texture.cpp
	common/texture.hpp
	common/geometry.hpp
	common/affine.hpp
	common/arcball.hpp)

target_link_libraries(Lab4 ${ALL_LIBS})

# Xcode and Visual Studio working directories
set_target_properties(Lab4 PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Lab4/")
create_target_launcher(Lab4 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Lab4/")

# Homework 5 skeleton
add_executable(Homework5_skeleton
	Homework5_skeleton/main.cpp
	Homework5_skeleton/VertexShader.glsl
	Homework5_skeleton/FragmentShader.glsl
	Homework5_skeleton/BumpVertexShader.glsl
	Homework5_skeleton/BumpFragmentShader.glsl
	Homework5_skeleton/EnvVertexShader.glsl
	Homework5_skeleton/EnvFragmentShader.glsl
	common/shader.cpp
	common/shader.hpp
	common/model.cpp
	common/model.hpp
	common/texture.cpp
	common/texture.hpp
	common/geometry.hpp
	common/affine.hpp
	common/arcball.hpp)

target_link_libraries(Homework5_skeleton ${ALL_LIBS})

# Xcode and Visual Studio working directories
set_target_properties(Homework5_skeleton PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Homework5_skeleton/")
create_target_launcher(Homework5_skeleton WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Homework5_skeleton/")
