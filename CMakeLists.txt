cmake_minimum_required(VERSION 3.20)

project(SimpleEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw glm)

add_library(glad_gl_core_33 STATIC
    external/glad/src/gl.c
)

target_include_directories(glad_gl_core_33 PUBLIC
    external/glad/include
)

find_package(OpenGL REQUIRED)

add_executable(SimpleEngine
    src/Application.cpp
    src/Mesh.cpp
    src/Renderer.cpp
    src/main.cpp
)

target_link_libraries(SimpleEngine PRIVATE
    glfw
    glad_gl_core_33
    OpenGL::GL
)

if(TARGET glm::glm)
    target_link_libraries(SimpleEngine PRIVATE glm::glm)
else()
    target_include_directories(SimpleEngine PRIVATE ${glm_SOURCE_DIR})
endif()
