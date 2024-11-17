cmake_minimum_required(VERSION 3.5)

project(ChessEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(
  ChessEngine
  src/main.cc
  src/piece.cc
  include/piece.h
  src/texture_factory.cc
  include/texture_factory.h
  include/square.h
  src/square.cc
  src/engine.cc
  include/engine.h
  src/board.cc
  include/board.h
  src/input_handler.cc
  include/input_handler.h
  src/renderer.cc
  include/renderer.h
  src/animation_engine.cc
  include/animation_engine.h)

add_subdirectory(dependencies)

target_link_libraries(ChessEngine ImGui-SFML::ImGui-SFML)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(ChessEngine PRIVATE IMGUI_MODE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(ChessEngine PRIVATE -O3)
endif()
