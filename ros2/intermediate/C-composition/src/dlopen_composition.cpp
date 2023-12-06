// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <memory>

#include "new_composition/talker_component.hpp"
#include <class_loader/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_components/node_factory.hpp>

int main(int argc, char **argv) {
  // Force flush of the stdout buffer.
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  auto args = rclcpp::init_and_remove_ros_arguments(argc, argv);
  // Initialize any global resources needed by the middleware and the client
  // library. This will also parse command line arguments one day (as of Beta 1
  // they are not used). You must call this before using any other part of the
  // ROS system. This should be called once per process. rclcpp::init(argc,
  // argv);

  // Create an executor that will be responsible for execution of callbacks for
  // a set of nodes. With this version, all callbacks will be called from within
  // this thread (the main one).
  rclcpp::executors::SingleThreadedExecutor exec;
  rclcpp::NodeOptions options;
  options.allow_undeclared_parameters(true)
      .automatically_declare_parameters_from_overrides(true);
  rclcpp::Logger logger = rclcpp::get_logger("new_composition");

  std::vector<std::unique_ptr<class_loader::ClassLoader>> loaders;
  std::vector<rclcpp_components::NodeInstanceWrapper> node_wrappers;
  std::vector<std::string> libraries;
  for (int i = 1; i < argc; ++i) {
    libraries.push_back(argv[i]);
  }
  for (auto library : libraries) {
    RCLCPP_INFO(logger, "Load library %s", library.c_str());
    auto loader = std::make_unique<class_loader::ClassLoader>(library);
    auto classes =
        loader->getAvailableClasses<rclcpp_components::NodeFactory>();
    for (auto clazz : classes) {
      RCLCPP_INFO(logger, "Instantiate class %s", clazz.c_str());
      auto node_factory =
          loader->createInstance<rclcpp_components::NodeFactory>(clazz);
      auto wrapper = node_factory->create_node_instance(options);
      auto node = wrapper.get_node_base_interface();
      node_wrappers.push_back(wrapper);
      exec.add_node(node);
    }
    loaders.push_back(std::move(loader));
  }

  // spin will block until work comes in, execute work as it becomes available,
  // and keep blocking. It will only be interrupted by Ctrl-C.
  exec.spin();

  for (auto wrapper : node_wrappers) {
    exec.remove_node(wrapper.get_node_base_interface());
  }
  node_wrappers.clear();

  rclcpp::shutdown();

  return 0;
}