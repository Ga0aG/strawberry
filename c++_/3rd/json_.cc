#include "../include/helper.hh"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

// sudo apt install nlohmann-json3-dev

int main() {
  // Read file
  std::ifstream config_stream("3rd/config.json");
  nlohmann::json config_json;
  config_stream >> config_json;
  // config_json = nlohmann::json::parse(config_stream);
  for (auto x : config_json.items()) {
    std::cout << now() << "Key: " << x.key() << ", Value: " << x.value()
              << std::endl;
  }
  { // Basic operation
    std::cout << now() << "***************************" << std::endl;
    std::cout << now() << "== Basic operation ==" << std::endl;
    std::cout << now() << "Read value by key: " << config_json["name"]
              << std::endl;
  }
}