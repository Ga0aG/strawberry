#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <iostream>

namespace py = pybind11;

void calculate_square_root(double value) {
    try {
        py::module math = py::module::import("math");
        double result = math.attr("sqrt")(value).cast<double>();
        std::cout << "Result: " << result << std::endl;
    } catch (const py::error_already_set &e) {
        std::cerr << "Python Error: " << e.what() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "C++ Error: " << e.what() << std::endl;
    }
}

int main() {
    py::scoped_interpreter guard{};  // 初始化解释器
    calculate_square_root(4);
    return 0;
}
