cmake_minimum_required(VERSION 3.13.2)
project(simple_math)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)

add_library(${PROJECT_NAME} SHARED
  src/sum.cpp)

target_include_directories(${PROJECT_NAME}
 PUBLIC
 $<INSTALL_INTERFACE:include>
 $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_executable(prog src/main.cpp)
target_include_directories(prog PUBLIC include)
target_link_libraries(prog ${PROJECT_NAME})

# message(${CMAKE_CURRENT_SOURCE_DIR})

install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

install(
  TARGETS simple_math
  EXPORT export_${PROJECT_NAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

install(TARGETS prog
    DESTINATION lib/${PROJECT_NAME})
