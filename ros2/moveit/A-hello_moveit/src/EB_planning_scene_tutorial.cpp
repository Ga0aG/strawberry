#include <rclcpp/rclcpp.hpp>

// MoveIt
#include <moveit/planning_scene/planning_scene.h>
#include <moveit/robot_model_loader/robot_model_loader.h>

#include <moveit/kinematic_constraints/utils.h>

// User defined constraints can also be specified to the PlanningScene class.
bool stateFeasibilityTestExample(
    const moveit::core::RobotState &kinematic_state, bool /*verbose*/) {
  const double *joint_values =
      kinematic_state.getJointPositions("panda_joint1");
  return (joint_values[0] > 0.0);
}

static const rclcpp::Logger LOGGER =
    rclcpp::get_logger("planning_scene_tutorial");

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions node_options;
  node_options.automatically_declare_parameters_from_overrides(true);
  auto planning_scene_tutorial_node =
      rclcpp::Node::make_shared("planning_scene_tutorial", node_options);

  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(planning_scene_tutorial_node);
  std::thread([&executor]() { executor.spin(); }).detach();

  // planning_scene can be easily setup and configured using a robot_model.
  // This is, however, not the recommended way to instantiate a PlanningScene.
  // **The planning_scene_monitor is the recommended method to create and
  // maintain the current planning scene** (and is discussed in detail in the
  // next tutorial) using data from the robot's joints and the sensors on the
  // robot. In this tutorial, we will instantiate a PlanningScene class
  // directly, but this method of instantiation is only intended for
  // illustration.

  robot_model_loader::RobotModelLoader robot_model_loader(
      planning_scene_tutorial_node, "robot_description");
  const moveit::core::RobotModelPtr &kinematic_model =
      robot_model_loader.getModel();
  planning_scene::PlanningScene planning_scene(kinematic_model);

  collision_detection::CollisionRequest collision_request;
  collision_detection::CollisionResult collision_result;
  moveit::core::RobotState &current_state =
      planning_scene.getCurrentStateNonConst();
  { // I. Self-collision checking
    planning_scene.checkSelfCollision(collision_request, collision_result);
    RCLCPP_INFO_STREAM(LOGGER,
                       "Test 1: Current state is "
                           << (collision_result.collision ? "in" : "not in")
                           << " self collision");
    // Change the state and check collision again
    current_state.setToRandomPositions();
    collision_result.clear();
    planning_scene.checkSelfCollision(collision_request, collision_result);
    RCLCPP_INFO_STREAM(LOGGER,
                       "Test 2: Current state is "
                           << (collision_result.collision ? "in" : "not in")
                           << " self collision");
  }

  { // II. Checking collision for a group
    // Now, we will do collision checking only for the hand of the
    // Panda, i.e. we will check whether there are any collisions between
    // the hand and other parts of the body of the robot.
    collision_request.group_name = "hand";
    current_state = planning_scene.getCurrentStateNonConst();
    current_state.setToRandomPositions();
    collision_result.clear();
    planning_scene.checkSelfCollision(collision_request, collision_result);
    RCLCPP_INFO_STREAM(LOGGER,
                       "Test 3: Current state is "
                           << (collision_result.collision ? "in" : "not in")
                           << " self collision");
  }

  const moveit::core::JointModelGroup *joint_model_group =
      current_state.getJointModelGroup("panda_arm");
  { // III. Getting Contact Information
    std::vector<double> joint_values = {0.0, 0.0, 0.0, -2.9, 0.0, 1.4, 0.0};
    current_state = planning_scene.getCurrentStateNonConst();
    current_state.setJointGroupPositions(joint_model_group, joint_values);
    RCLCPP_INFO_STREAM(LOGGER,
                       "Test 4: Current state is "
                           << (current_state.satisfiesBounds(joint_model_group)
                                   ? "valid"
                                   : "not valid"));

    // If true, compute contacts. Otherwise only a binary collision yes/no is
    // reported.
    collision_request.contacts = true;
    // Overall maximum number of contacts to compute
    collision_request.max_contacts = 1000;

    collision_result.clear();
    planning_scene.checkSelfCollision(collision_request, collision_result);
    RCLCPP_INFO_STREAM(LOGGER,
                       "Test 5: Current state is "
                           << (collision_result.collision ? "in" : "not in")
                           << " self collision");
    collision_detection::CollisionResult::ContactMap::const_iterator it;
    for (it = collision_result.contacts.begin();
         it != collision_result.contacts.end(); ++it) {
      RCLCPP_INFO(LOGGER, "Contact between: %s and %s", it->first.first.c_str(),
                  it->first.second.c_str());
    }
  }

  collision_detection::AllowedCollisionMatrix acm;
  moveit::core::RobotState copied_state = planning_scene.getCurrentState();
  { // IV. Modifying the Allowed Collision Matrix
    acm = planning_scene.getAllowedCollisionMatrix();

    collision_detection::CollisionResult::ContactMap::const_iterator it2;
    for (it2 = collision_result.contacts.begin();
         it2 != collision_result.contacts.end(); ++it2) {
      acm.setEntry(it2->first.first, it2->first.second, true);
    }
    collision_result.clear();
    planning_scene.checkSelfCollision(collision_request, collision_result,
                                      copied_state, acm);
    RCLCPP_INFO_STREAM(LOGGER,
                       "Test 6: Current state is "
                           << (collision_result.collision ? "in" : "not in")
                           << " self collision");
  }

  { // V. Full Collision Checking(include environment)
    // Note that collision checks with the environment will use the padded
    // version of the robot. Padding helps in keeping the robot further away
    // from obstacles in the environment.
    collision_result.clear();
    planning_scene.checkCollision(collision_request, collision_result,
                                  copied_state, acm);
    RCLCPP_INFO_STREAM(LOGGER,
                       "Test 7: Current state is "
                           << (collision_result.collision ? "in" : "not in")
                           << " self collision");
  }

  moveit_msgs::msg::Constraints goal_constraint;
  { // VI. Constraint Checking
    std::string end_effector_name =
        joint_model_group->getLinkModelNames().back();

    geometry_msgs::msg::PoseStamped desired_pose;
    desired_pose.pose.orientation.w = 1.0;
    desired_pose.pose.position.x = 0.3;
    desired_pose.pose.position.y = -0.185;
    desired_pose.pose.position.z = 0.5;
    desired_pose.header.frame_id = "panda_link0";
    // double tolerance_pos = 1e-3, double tolerance_angle = 1e-2
    goal_constraint = kinematic_constraints::constructGoalConstraints(
        end_effector_name, desired_pose);

    // Now, we can check a state against this constraint using the
    // isStateConstrained functions in the PlanningScene class.

    copied_state.setToRandomPositions();
    copied_state.update();
    bool constrained =
        planning_scene.isStateConstrained(copied_state, goal_constraint);
    RCLCPP_INFO_STREAM(
        LOGGER, "Test 8: Random state is "
                    << (constrained ? "constrained" : "not constrained"));
  }

  kinematic_constraints::KinematicConstraintSet kinematic_constraint_set(
      kinematic_model);
  { // VII. pre-processes constrain
    // There's a more efficient way of checking constraints (when you want
    // to check the same constraint over and over again, e.g. inside a
    // planner). We first construct a KinematicConstraintSet which
    // pre-processes the ROS Constraints messages and sets it up for quick
    // processing.
    kinematic_constraint_set.add(goal_constraint,
                                 planning_scene.getTransforms());
    bool constrained_2 = planning_scene.isStateConstrained(
        copied_state, kinematic_constraint_set);
    RCLCPP_INFO_STREAM(
        LOGGER, "Test 9: Random state is "
                    << (constrained_2 ? "constrained" : "not constrained"));

    // There's a direct way to do this using the KinematicConstraintSet
    // class.

    kinematic_constraints::ConstraintEvaluationResult constraint_eval_result =
        kinematic_constraint_set.decide(copied_state);
    RCLCPP_INFO_STREAM(LOGGER, "Test 10: Random state is "
                                   << (constraint_eval_result.satisfied
                                           ? "constrained"
                                           : "not constrained"));
  }

  { // VIII. User-defined constraints
    planning_scene.setStateFeasibilityPredicate(stateFeasibilityTestExample);
    bool state_feasible = planning_scene.isStateFeasible(copied_state);
    RCLCPP_INFO_STREAM(LOGGER,
                       "Test 11: Random state is "
                           << (state_feasible ? "feasible" : "not feasible"));

    // Whenever isStateValid is called, three checks are conducted: (a)
    // collision checking (b) constraint checking and (c) feasibility
    // checking using the user-defined callback.

    bool state_valid = planning_scene.isStateValid(
        copied_state, kinematic_constraint_set, "panda_arm");
    RCLCPP_INFO_STREAM(LOGGER, "Test 12: Random state is "
                                   << (state_valid ? "valid" : "not valid"));
  }

  rclcpp::shutdown();
  return 0;
}
