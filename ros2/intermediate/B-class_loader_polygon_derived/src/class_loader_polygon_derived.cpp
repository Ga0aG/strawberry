#include "class_loader_polygon_derived/class_loader_polygon_derived.hpp"
#include <class_loader_polygon_base/regular_polygon.hpp>
#include <cmath>

namespace class_loader_polygon_derived {

class Square : public polygon_base::RegularPolygon {
public:
  void initialize(double side_length) override {
    side_length_ = side_length;
    name = "Square";
  }

  double area() override { return side_length_ * side_length_; }

protected:
  double side_length_;
};

class Triangle : public polygon_base::RegularPolygon {
public:
  void initialize(double side_length) override {
    side_length_ = side_length;
    name = "Triangle";
  }

  double area() override { return 0.5 * side_length_ * getHeight(); }

  double getHeight() {
    return sqrt((side_length_ * side_length_) -
                ((side_length_ / 2) * (side_length_ / 2)));
  }

protected:
  double side_length_;
};

} // namespace class_loader_polygon_derived

#include <class_loader/register_macro.hpp>
CLASS_LOADER_REGISTER_CLASS(class_loader_polygon_derived::Square,
                            polygon_base::RegularPolygon)
CLASS_LOADER_REGISTER_CLASS(class_loader_polygon_derived::Triangle,
                            polygon_base::RegularPolygon)
