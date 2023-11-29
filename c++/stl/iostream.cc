#include "../include/helper.hh"
#include <fstream>
#include <ostream>
#include <string>

int main() {
  { // Read file
    print_header("Read file");
    std::ifstream config_stream("3rd/config.json");
    if (config_stream.good()) {
      std::string s;
      INFO_STREAM("Read file: ");
      while (std::getline(config_stream, s)) {
        std::cout << s;
      }
      std::cout << std::endl;
      config_stream.close();
    }
  }
  { // Write file
    print_header("Write file");
    std::ofstream file("3rd/config.json");
    file << "{\n  \"name\": \"Bamboo\"\n}\n";
    file.close();
  }
  { // stringstream
    print_header("Test StringStream");
    std::stringstream ss;
    int foo;
    ss << 100 << " " << 200;
    while (ss >> foo) {
      INFO_STREAM("" << foo);
    }
  }
}