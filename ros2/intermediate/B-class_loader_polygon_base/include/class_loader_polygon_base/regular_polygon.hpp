#ifndef POLYGON_BASE_REGULAR_POLYGON_HPP
#define POLYGON_BASE_REGULAR_POLYGON_HPP

#include <string>

namespace polygon_base {
class RegularPolygon {
public:
  virtual void initialize(double side_length) = 0;
  virtual double area() = 0;
  virtual ~RegularPolygon() {}
  std::string name = "";

protected:
  RegularPolygon() {}
};
} // namespace polygon_base

#endif // POLYGON_BASE_REGULAR_POLYGON_HPP