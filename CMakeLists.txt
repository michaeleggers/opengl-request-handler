cmake_minimum_required(VERSION 3.10)

project(Curltest VERSION 0.0.1)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(CURL)

add_executable(curltest
    main.cpp
)

target_link_libraries(curltest
    #${CMAKE_DL_LIBS}
    PUBLIC CURL::libcurl
    PUBLIC -lc -ldl -lpthread
)

# clangd compile_commands.json for lsp config
add_custom_command(
  TARGET curltest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_BINARY_DIR}/compile_commands.json
      ${CMAKE_SOURCE_DIR}/compile_commands.json || (exit 0)
)
