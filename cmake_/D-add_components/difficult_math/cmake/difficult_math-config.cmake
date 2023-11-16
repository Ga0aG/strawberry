# Report to users when this package has been found:
if(NOT @PROJECT_NAME@_FIND_QUIETLY)
  message(STATUS "Found @PROJECT_NAME@: @PROJECT_VERSION@ (${@PROJECT_NAME@_DIR})")
endif()

# Add support for `find_package(@PROJECT_NAME@)` as opposed to requiring each
# component to be specified manually:
if (NOT @PROJECT_NAME@_FIND_COMPONENTS)
  set(@PROJECT_NAME@_FIND_COMPONENTS @PROJECT_NAME@ sub multiply)
endif()

# Add support for `target_link_libraries(my_project @PROJECT_NAME@)`:
if (NOT TARGET @PROJECT_NAME@)
  add_library(@PROJECT_NAME@ INTERFACE IMPORTED)
endif()

# Remove already added components from the @PROJECT_NAME@_FIND_COMPONENTS list:
foreach(component ${@PROJECT_NAME@_FIND_COMPONENTS})
  if (TARGET @PROJECT_NAME@::${component})
    list(REMOVE_ITEM @PROJECT_NAME@_FIND_COMPONENTS ${component})
  endif()
endforeach()

message(${CMAKE_CURRENT_LIST_DIR})
foreach(component ${@PROJECT_NAME@_FIND_COMPONENTS})
  include(${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@-${component}-config.cmake)

  # Add support for `target_link_libraries(my_project @PROJECT_NAME@)`:
  target_link_libraries(@PROJECT_NAME@ INTERFACE @PROJECT_NAME@::${component})

  # Add support for `ament_target_dependencies(my_project @PROJECT_NAME@)`:
  get_property(definitions TARGET @PROJECT_NAME@::${component} PROPERTY INTERFACE_COMPILE_DEFINITIONS)
  get_property(libraries TARGET @PROJECT_NAME@::${component} PROPERTY INTERFACE_LINK_LIBRARIES)
  get_property(include_dirs TARGET @PROJECT_NAME@::${component} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
  set(@PROJECT_NAME@_DEFINITIONS "${@PROJECT_NAME@_DEFINITIONS};${definitions}")
  set(@PROJECT_NAME@_LIBRARIES "${@PROJECT_NAME@_LIBRARIES};${libraries}")
  set(@PROJECT_NAME@_INCLUDE_DIRS "${@PROJECT_NAME@_INCLUDE_DIRS};${include_dirs}")
endforeach()
