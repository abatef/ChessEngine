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
  include/restricted_piece.h
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

target_link_libraries(ChessEngine sfml-graphics sfml-window sfml-system
                      sfml-network)
