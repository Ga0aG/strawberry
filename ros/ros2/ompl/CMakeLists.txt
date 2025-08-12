cmake_minimum_required(VERSION 3.10.2)
project(ompl_tutorial)

find_package(ompl REQUIRED)
find_package(ament_cmake REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wno-overloaded-virtual)
endif()

include_directories(SYSTEM
                    ${Boost_INCLUDE_DIRS}
                    ${OMPL_INCLUDE_DIRS})
include_directories(include)


macro(build_demo demo_name src_file)
  add_executable(${demo_name} ${src_file})
  target_include_directories(${demo_name} PUBLIC ${OMPL_INCLUDE_DIRS})
target_link_libraries(${demo_name} PUBLIC ${OMPL_LIBRARIES})
  install(TARGETS ${demo_name} DESTINATION lib/${PROJECT_NAME}
)
endmacro(build_demo)

build_demo(rigid_body_planning src/A_RigidBodyPlanning.cpp)
build_demo(state_sampling src/B_StateSampling.cpp)
build_demo(benchmark src/C_KinematicChainBenchmark.cpp)
# build_demo(optimal_planning src/OptimalPlanning.cpp)
build_demo(constrained_planning src/D_ConstrainedPlanning.cpp)
build_demo(project_planning src/E_Projection.cpp)
build_demo(multilevel_planning src/F_MultiLevelPlanningKinematicChain.cpp)

ament_package()
