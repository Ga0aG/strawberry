cmake_minimum_required(VERSION 3.5)
project(new_composition)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(class_loader REQUIRED)
find_package(std_msgs REQUIRED)

include_directories(include)

# create ament index resource which references the libraries in the binary dir
set(node_plugins "")

add_library(talker_component SHARED
  src/talker_component.cpp)
ament_target_dependencies(talker_component
  rclcpp
  rclcpp_components
  class_loader
  std_msgs)
rclcpp_components_register_nodes(talker_component "composition::Talker")
# set(node_plugins "${node_plugins}composition::Talker;$<TARGET_FILE:talker_component>\n")

add_library(listener_component SHARED
  src/listener_component.cpp)
ament_target_dependencies(listener_component
  rclcpp
  rclcpp_components
  class_loader
  std_msgs)
rclcpp_components_register_nodes(listener_component "composition::Listener")
# set(node_plugins "${node_plugins}composition::Listener;$<TARGET_FILE:listener_component>\n")

add_executable(manual_composition
  src/manual_composition.cpp)
ament_target_dependencies(manual_composition
  rclcpp
  )
target_link_libraries(manual_composition
  talker_component
  listener_component)

add_executable(dlopen_composition
  src/dlopen_composition.cpp)
ament_target_dependencies(dlopen_composition
  rclcpp
  class_loader
  rclcpp_components
  )

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # uncomment the line when a copyright and license is not present in all source files
  #set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # uncomment the line when this package is not in a git repo
  #set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

install(TARGETS
  talker_component
  listener_component
  ARCHIVE DESTINATION lib/
  LIBRARY DESTINATION lib/
  RUNTIME DESTINATION bin)

install(TARGETS
  manual_composition
  dlopen_composition
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY
  include/
  DESTINATION include)

# install(DIRECTORY
#   launch config
#   DESTINATION share/${PROJECT_NAME})

ament_export_libraries(${PROJECT_NAME})
ament_export_include_directories(include)
ament_export_dependencies(${DEP_PACKAGES})

ament_package()
