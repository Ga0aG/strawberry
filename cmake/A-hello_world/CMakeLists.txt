cmake_minimum_required(VERSION 3.5)
project(simple_math)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_CURRENT_SOURCE_DIR}/install)
# set(INCLUDE_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/install/include)
# Same result in *.cmake as above
set(INCLUDE_INSTALL_DIR include)

add_library(${PROJECT_NAME} SHARED
  src/sum.cpp)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
 PUBLIC
 $<INSTALL_INTERFACE:include>
 $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_executable(prog src/main.cpp)
# target_include_directories(prog PUBLIC include)
target_link_libraries(prog ${PROJECT_NAME})

# message(${CMAKE_CURRENT_SOURCE_DIR})

# Install includes
install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

# Install lib
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install executable
install(TARGETS prog
    DESTINATION lib/${PROJECT_NAME})

# Export targets. Create imported target simple_math::simple_math # colcon no need
install(
  EXPORT ${PROJECT_NAME}-targets
  FILE "${PROJECT_NAME}-targets.cmake"
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION lib/cmake/${PROJECT_NAME}
)

# Install CMake files: Use for find package # colcon no need
configure_file(
  "cmake/${PROJECT_NAME}-config.cmake"
  "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
  @ONLY
)
install(
  FILES
    "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
  DESTINATION
    "lib/cmake/${PROJECT_NAME}"
)

# Generate version file.
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-version.cmake"
  VERSION 0.0.0
  COMPATIBILITY AnyNewerVersion
)

install(
  FILES
  "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-version.cmake"
  DESTINATION lib/cmake/${PROJECT_NAME}
)