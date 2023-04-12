cmake_minimum_required(VERSION 3.0.0)
project(KAnime+ VERSION 0.1.0)

set(
  PROG_SRCS

  src/main.cpp
  src/json/jFunc.cpp

  src/mainWindow/mainWindow.cpp
  src/searchDialog/searchDialog.cpp
  src/comm/comm.cpp
)

set(CMAKE_CXX_STANDARD 17)

find_package(wxWidgets COMPONENTS core base xrc REQUIRED)
include(${wxWidgets_USE_FILE})

add_executable(${PROJECT_NAME} ${PROG_SRCS})
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} curl pthread)
