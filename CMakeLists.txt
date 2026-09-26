cmake_minimum_required(VERSION 3.12)

project(
  labwork10
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_library(labwork10_lib
  src/dispatcher.cpp
  src/kernel.cpp
  src/parser.cpp
)

target_include_directories(labwork10_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(labwork10 ./bin/main.cpp)
target_link_libraries(labwork10 PRIVATE labwork10_lib)

add_subdirectory(tests)