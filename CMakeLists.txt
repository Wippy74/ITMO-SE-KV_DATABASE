cmake_minimum_required(VERSION 3.12)

project(
  ./valkey
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(labwork10_lib
  src/dispatcher.cpp
  src/kernel.cpp
  src/parser.cpp
)

target_include_directories(labwork10_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(valkey ./bin/main.cpp)
target_link_libraries(valkey PRIVATE labwork10_lib)