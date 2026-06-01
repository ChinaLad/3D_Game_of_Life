cmake_minimum_required(VERSION 3.20)
project(3D_Game_of_Life LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++")
set(CMAKE_CXX_FLAGS "-O3  -DGLM_ENABLE_EXPERIMENTAL")

# 1. Find the packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# 2. Add your executable
set_source_files_properties(gl.c PROPERTIES LANGUAGE CXX)
add_executable(3D_Game_of_Life main.cpp
        gl.c
        vbo.h
        shader.h
        ebo.h
        vao.h
        stb_image.cpp
        stb_image.h
        texture3d.h
)

target_compile_definitions(3D_Game_of_Life PRIVATE GLM_ENABLE_EXPERIMENTAL)

# 3. Link the libraries to your project
target_link_libraries(3D_Game_of_Life
        PUBLIC
        OpenGL::GL
        glfw
)