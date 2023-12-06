#include <class_loader/class_loader.hpp>
#include <class_loader_polygon_base/regular_polygon.hpp>
#include <exception>
#include <memory>
#include <rclcpp/rclcpp.hpp>

int main(int argc, char **argv) {
  auto args = rclcpp::init_and_remove_ros_arguments(argc, argv);
  auto loader = std::make_unique<class_loader::ClassLoader>(args[1]);
  std::vector<std::string> classes =
      loader->getAvailableClasses<polygon_base::RegularPolygon>();
  if (classes.empty()) {
    printf("%s is empty\n", args[1].c_str());
  }
  for (unsigned int c = 0; c < classes.size(); ++c) {
    std::shared_ptr<polygon_base::RegularPolygon> plugin =
        loader->createInstance<polygon_base::RegularPolygon>(classes[c]);
    plugin->initialize(10.0);
    printf("%s, area: %.2f\n", classes[c].c_str(), plugin->area());
  }

  return 0;
}