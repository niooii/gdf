cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_COMPILER clang++)
project(GameDevFinal
    VERSION 1.0
    DESCRIPTION "Not really a game dev final..."
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_program(GLSLC glslc REQUIRED)
add_subdirectory(gdfe)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# defaut build is debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

set(NO_WARNING_FLAGS "-Wno-unused-parameter -Wno-missing-braces -Wno-unused-variable -Wno-ignored-qualifiers -Wno-missing-field-initializers")

set(CMAKE_CXX_FLAGS "-Wall -W -Wextra ${NO_WARNING_FLAGS} -ffast-math -march=native -mavx2 -fcolor-diagnostics -fno-exceptions")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(PROFILE_DIR "release")
    # TODO! link time optimization BREAKS EVERYTRHING
    #  -flto
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -fno-plt -fuse-ld=lld -funroll-loops -finline-functions -fomit-frame-pointer -fno-exceptions -ftree-vectorize -fvectorize -fslp-vectorize -DNDEBUG -DGDF_RELEASE")
else()
    set(PROFILE_DIR "debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0 -fno-omit-frame-pointer -DDEBUG")
endif()

function(find_src_files result search_dir)
    file(GLOB_RECURSE source_files
            "${search_dir}/*.cpp"
            "${search_dir}/*.c"
    )
    set(${result} ${source_files} PARENT_SCOPE)
endfunction()

find_src_files(SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src")
find_src_files(EXTERN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/extern")

SET_SOURCE_FILES_PROPERTIES(${SOURCE_FILES} ${EXTERN_SRC} PROPERTIES LANGUAGE CXX)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${EXTERN_SRC})

if(DEFINED ENV{VULKAN_SDK})
    set(Vulkan_INCLUDE_DIRS "$ENV{VULKAN_SDK}/include")
    set(Vulkan_LIBRARIES "$ENV{VULKAN_SDK}/lib")
    include_directories(${Vulkan_INCLUDE_DIRS})
    link_directories(${Vulkan_LIBRARIES})
else()
    find_package(Vulkan REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            Vulkan::Vulkan
    )
endif()

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        gdfe
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern"
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${PROFILE_DIR}/"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME 
        DESTINATION $<$<CONFIG:Debug>:debug>$<$<CONFIG:Release>:release>
    LIBRARY 
        DESTINATION $<$<CONFIG:Debug>:debug/lib>$<$<CONFIG:Release>:release/lib>
    ARCHIVE 
        DESTINATION $<$<CONFIG:Debug>:debug/lib>$<$<CONFIG:Release>:release/lib>
)

function(find_shader_files SHADER_FILES)
    file(GLOB VERT_SHADERS "${CMAKE_SOURCE_DIR}/resources/shaders/*.vert")
    file(GLOB FRAG_SHADERS "${CMAKE_SOURCE_DIR}/resources/shaders/*.frag")
    set(${SHADER_FILES} ${VERT_SHADERS} ${FRAG_SHADERS} PARENT_SCOPE)
endfunction()

function(get_spv_output_path SHADER_PATH OUTPUT_PATH)
    get_filename_component(SHADER_NAME ${SHADER_PATH} NAME)
        
    # Construct the output path in the build directory
    set(SPV_PATH "${CMAKE_BINARY_DIR}/${PROFILE_DIR}/resources/shaders/${SHADER_NAME}.spv")

    # Set the result variable in the parent scope
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
        ${CMAKE_BINARY_DIR}/${PROFILE_DIR}/resources
)

add_dependencies(compile_shaders copy_resources)
add_dependencies(${PROJECT_NAME} compile_shaders)