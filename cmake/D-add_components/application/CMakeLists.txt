cmake_minimum_required(VERSION 3.5)
project(use_custom_pack)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../difficult_math/install")

find_package(difficult_math REQUIRED COMPONENTS sub)
add_executable(prog src/main.cpp)
target_link_libraries(prog PUBLIC difficult_math::sub)

install(TARGETS prog
    DESTINATION lib/${PROJECT_NAME})
