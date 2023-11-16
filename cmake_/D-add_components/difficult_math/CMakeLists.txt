cmake_minimum_required(VERSION 3.5)
project(difficult_math)

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

add_library(sub SHARED
  src/sub.cpp)
add_library(${PROJECT_NAME}::sub ALIAS sub)

target_include_directories(sub
 PUBLIC
 $<INSTALL_INTERFACE:include>
 $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_library(multiply SHARED
  src/multiply.cpp)
add_library(${PROJECT_NAME}::multiply ALIAS multiply)

target_include_directories(multiply
 PUBLIC
 $<INSTALL_INTERFACE:include>
 $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Install includes
install(
  DIRECTORY include/
  DESTINATION include
)

# Install sub component lib
install(
  TARGETS sub
  EXPORT sub-targets
  LIBRARY DESTINATION lib
)

# Install multiply component lib
install(
  TARGETS multiply
  EXPORT multiply-targets
  LIBRARY DESTINATION lib
)

# Export sub targets
install(
  EXPORT sub-targets
  FILE "${PROJECT_NAME}-sub-targets.cmake"
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION lib/cmake/${PROJECT_NAME}
)

# Export multiply targets
install(
  EXPORT multiply-targets
  FILE "${PROJECT_NAME}-multiply-targets.cmake"
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

# Install sub component CMake files:
configure_file(
  "cmake/${PROJECT_NAME}-sub-config.cmake"
  "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-sub-config.cmake"
  @ONLY
)
install(
  FILES
    "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-sub-config.cmake"
  DESTINATION
    "lib/cmake/${PROJECT_NAME}"
)

# Install camera3d_interface component CMake files:
configure_file(
  "cmake/${PROJECT_NAME}-multiply-config.cmake"
  "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-multiply-config.cmake"
  @ONLY
)
install(
  FILES
    "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-multiply-config.cmake"
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