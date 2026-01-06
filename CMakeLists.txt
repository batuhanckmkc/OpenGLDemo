cmake_minimum_required(VERSION 3.10)
project(OpenGLDemo)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
set(CMAKE_PREFIX_PATH "/opt/homebrew") 
find_package(glfw3 REQUIRED)

include_directories(include)
add_executable(OpenGLDemo main.cpp glad.c)
target_link_libraries(OpenGLDemo PUBLIC OpenGL::GL glfw)