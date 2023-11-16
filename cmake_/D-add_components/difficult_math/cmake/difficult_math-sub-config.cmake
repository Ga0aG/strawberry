if(NOT TARGET @PROJECT_NAME@::sub)

  include(CMakeFindDependencyMacro)

  # find_dependency(dr_common REQUIRED)
  include(${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@-sub-targets.cmake)
endif()
