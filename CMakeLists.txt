cmake_minimum_required(VERSION 3.10)

project(Curltest VERSION 0.0.1)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set(CMAKE_VERBOSE_MAKEFILE ON)

#find_package(CURL)

add_subdirectory(dependencies/SDL3)

    find_package(glslang CONFIG REQUIRED)

add_executable(curltest
    main.cpp
    shader.cpp
    platform.cpp
    utils.cpp
    dependencies/glad/src/glad.c
)

target_link_libraries(curltest
    #${CMAKE_DL_LIBS}
    #PUBLIC CURL::libcurl
    PUBLIC -lc -ldl -lpthread -lwayland-client
    PUBLIC SDL3::SDL3
    PUBLIC glslang::glslang
    PUBLIC glslang::SPIRV 
    PUBLIC glslang::glslang-default-resource-limits
)

target_include_directories(curltest PUBLIC
    dependencies/glad/include/
)

# clangd compile_commands.json for lsp config
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_custom_command(
  TARGET curltest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_BINARY_DIR}/compile_commands.json
      ${CMAKE_SOURCE_DIR}/compile_commands.json || (exit 0)
)
