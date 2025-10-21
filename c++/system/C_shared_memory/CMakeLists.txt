cmake_minimum_required(VERSION 3.10)
project(SystemExamples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required libraries
find_library(RT_LIBRARY rt)
if(RT_LIBRARY)
    set(SYSTEM_LIBS ${RT_LIBRARY})
else()
    message(WARNING "librt not found, shared memory examples may not link properly")
endif()

# Shared memory example
add_executable(shm_writer shm_writer.cpp)
target_link_libraries(shm_writer ${SYSTEM_LIBS})

add_executable(shm_reader shm_reader.cpp)
target_link_libraries(shm_reader ${SYSTEM_LIBS})

# Other system examples (if they exist)
if(EXISTS "epoll_server.cc")
    add_executable(epoll_server epoll_server.cc)
    target_link_libraries(epoll_server ${SYSTEM_LIBS})
endif()

if(EXISTS "epoll_client.cc")
    add_executable(epoll_client epoll_client.cc)
    target_link_libraries(epoll_client ${SYSTEM_LIBS})
endif()

if(EXISTS "epoll_client_improve.cc")
    add_executable(epoll_client_improve epoll_client_improve.cc)
    target_link_libraries(epoll_client_improve ${SYSTEM_LIBS})
endif()

# Print build information
message(STATUS "System examples will be built:")
message(STATUS "  - shm_writer: POSIX shared memory writer")
message(STATUS "  - shm_reader: POSIX shared memory reader")
if(EXISTS "epoll_server.cc")
    message(STATUS "  - epoll_server: epoll-based server")
endif()
if(EXISTS "epoll_client.cc")
    message(STATUS "  - epoll_client: epoll-based client")
endif()
if(EXISTS "epoll_client_improve.cc")
    message(STATUS "  - epoll_client_improve: improved epoll client")
endif()
