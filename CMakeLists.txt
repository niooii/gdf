cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_COMPILER clang++)
project(gdf
        VERSION 1.0
        DESCRIPTION "Not really a game dev final..."
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_program(GLSLC glslc REQUIRED)
add_subdirectory(gdfe)
add_subdirectory(extern/ser20)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# default build is debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

set(NO_WARNING_FLAGS "-Wno-unused-parameter -Wno-missing-braces -Wno-unused-variable -Wno-ignored-qualifiers -Wno-missing-field-initializers")

set(CMAKE_CXX_FLAGS "-Wall -W -Wextra ${NO_WARNING_FLAGS} -ffast-math -march=native -mavx2 -fcolor-diagnostics")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
#    set(PROFILE_DIR "release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -fno-plt -fuse-ld=lld -funroll-loops -finline-functions -fomit-frame-pointer -ftree-vectorize -fvectorize -fslp-vectorize -DNDEBUG -DGDF_RELEASE")
else()
#    set(PROFILE_DIR "debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0 -fno-omit-frame-pointer -DDEBUG")
endif()

function(find_src_files result search_dir)
    file(GLOB_RECURSE source_files
            "${search_dir}/*.cpp"
            "${search_dir}/*.c"
    )
    set(${result} ${source_files} PARENT_SCOPE)
endfunction()

find_src_files(GAME_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/game")
find_src_files(CLIENT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/client")
find_src_files(SERVER_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/server")
find_src_files(EXTERN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/extern")

SET_SOURCE_FILES_PROPERTIES(${GAME_SRC} ${CLIENT_SRC} ${SERVER_SRC} ${EXTERN_SRC} PROPERTIES LANGUAGE CXX)

add_library(gdf_common STATIC ${GAME_SRC} ${EXTERN_SRC}
        include/game/events/defs.h
        include/game/events/defs.h)

# TODO! remove later, server src for now is for easy development
add_executable(client ${CLIENT_SRC} ${SERVER_SRC}
        include/constants.h
        include/game/events/defs.h)
target_compile_definitions(client PRIVATE GDF_CLIENT_BUILD)

add_executable(server ${SERVER_SRC}
        include/game/events/defs.h
        include/game/events/defs.h)

if(DEFINED ENV{VULKAN_SDK})
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(Vulkan_INCLUDE_DIRS "$ENV{VULKAN_SDK}/Include")
    else()
        set(Vulkan_INCLUDE_DIRS "$ENV{VULKAN_SDK}/include")
    endif()
    include_directories(${Vulkan_INCLUDE_DIRS})
    find_package(Vulkan REQUIRED)
else()
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIRS})
endif()

target_include_directories(gdf_common
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern"
        "${CMAKE_CURRENT_SOURCE_DIR}/game"
)

target_link_libraries(gdf_common
        PUBLIC
        Vulkan::Vulkan
        gdfe
        ser20
)

target_link_libraries(client
        PRIVATE
        gdf_common
)

target_link_libraries(server
        PRIVATE
        gdf_common
)

foreach(TARGET gdf_common client server)
    set_target_properties(${TARGET}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
endforeach()

function(find_shader_files SHADER_FILES)
    file(GLOB VERT_SHADERS "${CMAKE_SOURCE_DIR}/resources/shaders/*.vert")
    file(GLOB FRAG_SHADERS "${CMAKE_SOURCE_DIR}/resources/shaders/*.frag")
    set(${SHADER_FILES} ${VERT_SHADERS} ${FRAG_SHADERS} PARENT_SCOPE)
endfunction()

function(get_spv_output_path SHADER_PATH OUTPUT_PATH)
    get_filename_component(SHADER_NAME ${SHADER_PATH} NAME)

    set(SPV_PATH "${CMAKE_BINARY_DIR}/resources/shaders/${SHADER_NAME}.spv")

    set(${OUTPUT_PATH} ${SPV_PATH} PARENT_SCOPE)
endfunction()

add_custom_target(compile_shaders)

find_shader_files(SHADER_FILES)

foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_EXT ${SHADER} EXT)
    string(SUBSTRING ${SHADER_EXT} 1 -1 SHADER_STAGE)

    get_spv_output_path(${SHADER} SPV_OUTPUT)

    get_filename_component(OUTPUT_DIR ${SPV_OUTPUT} DIRECTORY)
    file(MAKE_DIRECTORY ${OUTPUT_DIR})

    add_custom_command(
            TARGET compile_shaders
            COMMAND glslc
            ARGS -fshader-stage=${SHADER_STAGE} ${SHADER} -o ${SPV_OUTPUT}
            DEPENDS ${SHADER}
            VERBATIM
    )
endforeach()

add_custom_target(copy_resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_BINARY_DIR}/resources
)

add_dependencies(compile_shaders copy_resources)
add_dependencies(client compile_shaders)
