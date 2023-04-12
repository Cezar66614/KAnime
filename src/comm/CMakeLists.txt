cmake_minimum_required(VERSION 3.0.0)
project(Comm VERSION 0.1.0)

set(
  PROG_SRCS

  main.cpp
  comm.cpp
)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} ${PROG_SRCS})
target_link_libraries(${PROJECT_NAME} curl pthread)
