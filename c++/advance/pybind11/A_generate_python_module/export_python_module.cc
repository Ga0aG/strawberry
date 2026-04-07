#include <pybind11/pybind11.h>
#include <cmath>

namespace py = pybind11;

double add(double a, double b) {
    return a + b;
}

double square_root(double x) {
    return std::sqrt(x);
}

// 创建模块
PYBIND11_MODULE(example, m) {
    m.def("add", &add, "A function that adds two numbers");
    m.def("square_root", &square_root, "A function that calculates the square root of a number");
}

// g++ -O3 -Wall -shared -std=c++11 -fPIC $(python3 -m pybind11 --includes) export_python_module.cc -o example$(python3-config --extension-suffix)
