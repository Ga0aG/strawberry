if(NOT TARGET @PROJECT_NAME@::multiply)

  include(CMakeFindDependencyMacro)

  # find_dependency(dr_common REQUIRED)
  include(${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@-multiply-targets.cmake)
endif()
