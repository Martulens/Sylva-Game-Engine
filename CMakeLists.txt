cmake_minimum_required(VERSION 3.20)
project(Sylva LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(MSVC)
    add_compile_options(/W3 /permissive- /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX)
else()
    add_compile_options(-Wall -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare)
endif()

find_package(OpenGL REQUIRED)

include(FetchContent)

FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND  OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11      ON  CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.6
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  cmake)
FetchContent_MakeAvailable(glad)
glad_add_library(glad_gl_core_33 STATIC REPRODUCIBLE API gl:core=3.3)

FetchContent_Declare(assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v5.0.1
    GIT_SHALLOW    TRUE)
set(ASSIMP_BUILD_TESTS                    OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS             OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES                  OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL                        OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL_PDB                    OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS             OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER             ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER             ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_COLLADA_IMPORTER         ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER            ON  CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS                     OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)

FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(stb)
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_SOURCE_DIR})

set(SYLVA_SOURCES
    src/main.cpp
    src/camera.cpp
    src/gamestate.cpp
    src/meshgeometry.cpp
    src/modeltexture.cpp
    src/noise.cpp
    src/objects.cpp
    src/render.cpp
    src/shaders.cpp
    src/terrain.cpp
    src/sylva/sylva.cpp)

add_executable(sylva ${SYLVA_SOURCES})
target_include_directories(sylva PRIVATE src)
target_link_libraries(sylva PRIVATE
    glfw
    glad_gl_core_33
    glm::glm
    assimp
    stb_image
    OpenGL::GL)

if(APPLE)
    target_compile_definitions(sylva PRIVATE GL_SILENCE_DEPRECATION)
endif()

set(SYLVA_ASSET_DIRS models textures)
foreach(dir ${SYLVA_ASSET_DIRS})
    add_custom_command(TARGET sylva POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/src/${dir}
            $<TARGET_FILE_DIR:sylva>/${dir})
endforeach()

file(GLOB SYLVA_SHADERS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/shaders/*.vert
    ${CMAKE_SOURCE_DIR}/src/shaders/*.frag)
foreach(shader ${SYLVA_SHADERS})
    get_filename_component(shader_name ${shader} NAME)
    add_custom_command(TARGET sylva POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${shader}
            $<TARGET_FILE_DIR:sylva>/${shader_name})
endforeach()
