cmake_minimum_required(VERSION 2.6)
project(diffusion_curves)
SET(GCC_COMPILE_FLAGS "-g")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
add_definitions(${GCC_COMPILE_FLAGS})

### libIGL options: choose between header only and compiled static library
option(LIBIGL_USE_STATIC_LIBRARY     "Use libigl as static library" OFF)
option(LIBIGL_WITH_OPENGL            "Use OpenGL"         ON)
option(LIBIGL_WITH_OPENGL_GLFW       "Use GLFW"           ON)

find_package(LIBIGL REQUIRED)

find_package( OpenCV )
if (OpenCV_FOUND)
  include_directories(${OpenCV_INCLUDE_DIRS})
endif()

# Add your project files
include_directories("include/")
if(USE_SOLUTION)
  file(GLOB SRCFILES solution/*.cpp)
else()
  file(GLOB SRCFILES src/*.cpp src/*/*.cpp)
endif()

add_library(core ${SRCFILES})
target_link_libraries(core igl::core igl::opengl igl::opengl_glfw )

add_executable(diffusion_curves "main.cpp")
target_link_libraries(diffusion_curves core igl::core igl::opengl igl::opengl_glfw ${OpenCV_LIBS})
