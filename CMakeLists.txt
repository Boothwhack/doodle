cmake_minimum_required(VERSION 3.30)
project(doodle)

set(CMAKE_CXX_STANDARD 23)

include(ExternalProject)
include(FetchContent)

# --- Load 3rd party libraries

FetchContent_Declare(GLFW3
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG 3.4
        GIT_SHALLOW
        # attempt to use system install first
        FIND_PACKAGE_ARGS 3.4
)
FetchContent_MakeAvailable(GLFW3)

# fetch GLAD
FetchContent_Declare(glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad
        GIT_TAG v2.0.8
        GIT_SHALLOW
        SOURCE_SUBDIR cmake
)
FetchContent_MakeAvailable(glad)

# fetch TOML++
FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

# fetch GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG "1.0.1"
)
FetchContent_MakeAvailable(glm)

find_package(OpenGL REQUIRED)

glad_add_library(glad REPRODUCIBLE EXCLUDE_FROM_ALL LOADER API gl:core=4.6)

# --- Application executable

add_executable(doodle
        doodle/gl.cpp
        doodle/gl.h
        doodle/main.cpp
)
target_link_libraries(doodle PUBLIC glfw glad tomlplusplus::tomlplusplus glm::glm)
