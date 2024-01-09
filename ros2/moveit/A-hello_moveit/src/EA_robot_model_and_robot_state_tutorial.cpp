/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2012, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/* Author: Sachin Chitta, Michael Lautman*/

// MoveIt
#include <moveit/robot_model/robot_model.h>
#include <moveit/robot_model_loader/robot_model_loader.h>
#include <moveit/robot_state/robot_state.h>

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions node_options;
  node_options.automatically_declare_parameters_from_overrides(true);
  auto node =
      rclcpp::Node::make_shared("robot_model_and_state_tutorial", node_options);
  const auto &LOGGER = node->get_logger();

  // `RobotModelLoader` will look up the robot description on the ROS parameter
  // server and construct a moveit_codedir?
  robot_model_loader::RobotModelLoader robot_model_loader(node);
  const moveit::core::RobotModelPtr &kinematic_model =
      robot_model_loader.getModel();
  RCLCPP_INFO(LOGGER, "Model frame: %s",
              kinematic_model->getModelFrame().c_str());

  moveit::core::RobotStatePtr kinematic_state(
      new moveit::core::RobotState(kinematic_model));
  kinematic_state->setToDefaultValues();
  const moveit::core::JointModelGroup *joint_model_group =
      kinematic_model->getJointModelGroup("panda_arm");

  const std::vector<std::string> &joint_names =
      joint_model_group->getVariableNames();

  std::vector<double> joint_values;
  { // I. Get Joint Values
    kinematic_state->copyJointGroupPositions(joint_model_group, joint_values);
    for (std::size_t i = 0; i < joint_names.size(); ++i) {
      RCLCPP_INFO(LOGGER, "Joint %s: %f", joint_names[i].c_str(),
                  joint_values[i]);
    }
  }

  { // II. Joint Limits
    /* Set one joint in the Panda arm outside its joint limit */
    joint_values[0] = 5.57;
    kinematic_state->setJointGroupPositions(joint_model_group, joint_values);

    /* Check whether any joint is outside its joint limits */
    RCLCPP_INFO_STREAM(
        LOGGER,
        "Current state is "
            << (kinematic_state->satisfiesBounds() ? "valid" : "not valid")
            << "; Jonint 1: " << joint_values[0]);

    /* Enforce the joint limits for this state and check again*/
    kinematic_state->enforceBounds();
    kinematic_state->copyJointGroupPositions(joint_model_group, joint_values);
    RCLCPP_INFO_STREAM(
        LOGGER,
        "Current state is "
            << (kinematic_state->satisfiesBounds() ? "valid" : "not valid")
            << "; Jonint 1: " << joint_values[0]);
  }

  { // III. Forward Kinematics
    kinematic_state->setToRandomPositions(joint_model_group);
    const Eigen::Isometry3d &end_effector_state =
        kinematic_state->getGlobalLinkTransform("panda_link8");

    /* Print end-effector pose. Remember that this is in the model frame */
    RCLCPP_INFO_STREAM(LOGGER, "Translation: \n"
                                   << end_effector_state.translation() << "\n");
    RCLCPP_INFO_STREAM(LOGGER, "Rotation: \n"
                                   << end_effector_state.rotation() << "\n");
  }

  { // IV. Inverse Kinematics
    double timeout = 0.1;
    const Eigen::Isometry3d &end_effector_state =
        kinematic_state->getGlobalLinkTransform("panda_link8");
    bool found_ik = kinematic_state->setFromIK(joint_model_group,
                                               end_effector_state, timeout);

    // Now, we can print out the IK solution (if found):
    if (found_ik) {
      kinematic_state->copyJointGroupPositions(joint_model_group, joint_values);
      for (std::size_t i = 0; i < joint_names.size(); ++i) {
        RCLCPP_INFO(LOGGER, "Joint %s: %f", joint_names[i].c_str(),
                    joint_values[i]);
      }
    } else {
      RCLCPP_INFO(LOGGER, "Did not find IK solution");
    }
  }

  { // V. Get the Jacobian
    Eigen::Vector3d reference_point_position(0.0, 0.0, 0.0);
    Eigen::MatrixXd jacobian;
    kinematic_state->getJacobian(
        joint_model_group,
        kinematic_state->getLinkModel(
            joint_model_group->getLinkModelNames().back()),
        reference_point_position, jacobian);
    RCLCPP_INFO_STREAM(LOGGER, "Jacobian: \n" << jacobian << "\n");
  }

  { // [M] Test joint model group
    // Print information of joint in hands
    const moveit::core::JointModelGroup *joint_model_group_hand =
        kinematic_state->getJointModelGroup("hand");

    auto print_joint_positions = [&kinematic_state, &joint_model_group_hand,
                                  &LOGGER]() {
      std::vector<double> joint_values_hand;
      const std::vector<std::string> &joint_names_hand =
          joint_model_group_hand->getVariableNames();
      kinematic_state->copyJointGroupPositions(joint_model_group_hand,
                                               joint_values_hand);
      RCLCPP_INFO_STREAM(LOGGER, "Print hand joint positions:");
      for (unsigned int ind = 0; ind < joint_names_hand.size(); ++ind) {
        RCLCPP_INFO_STREAM(LOGGER, "Joint " << joint_names_hand[ind]
                                            << ", Position: "
                                            << joint_values_hand[ind]);
      }
    };
    print_joint_positions();
    kinematic_state->setToRandomPositions(joint_model_group_hand);
    print_joint_positions();
    // compute ik for hand group
  }

  rclcpp::shutdown();
  return 0;
}
