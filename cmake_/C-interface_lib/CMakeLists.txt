cmake_minimum_required(VERSION 3.5)
project(happy_math)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../A-hello_world/install")
find_package(simple_math REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME}
INTERFACE # target_include_directories may only set INTERFACE properties on INTERFACE
 $<INSTALL_INTERFACE:include>
 $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
# INTERFACE library can only be used with the INTERFACE keyword of target_link_libraries
# target_link_libraries(${PROJECT_NAME} INTERFACE simple_math::simple_math)

add_executable(prog src/main.cpp)
target_link_libraries(prog PUBLIC ${PROJECT_NAME})
