#include "../include/helper.hh"
#include <string>

// I. Access modifiers
class Base {
public:
  int x_ = 0;
  void show() { INFO_STREAM("Show time"); }

  // protected member can be accessed by derived class
protected:
  int y_ = 0;

  // private member can only accessed by Bass class itself and friend class
private:
  int z_ = 0;
};
class PublicDerived : public Base {
  // x is public
  // y is protected
  // z is not accessible from PublicDerived
};
class ProtectedDerived : protected Base {
  // x is protected
  // y is protected
  // z is not accessible from ProtectedDerived
};
// Private is the default access specifier
class PrivateDerived : private Base {
  // x is private
  // y is private
  // z is not accessible from PrivateDerived
};

class Shape {
public:
  // II. Constructors
  // Constructor with argument
  Shape(const std::string &name) : name_(name) {
    INFO_STREAM("Init shape: " << name_);
  }
  // Copy constructor
  Shape(const Shape &shape) {
    name_ = shape.name_;
    INFO_STREAM("Copy construct shape: " << shape.name_);
  }
  // Move constructor
  // todo
  // Deconstructor
  ~Shape() { INFO_STREAM("Bye shape: " << name_); }
  // III. Virtual function
  // Pure virtual function, derived class must override
  virtual int area() const = 0;
  // Const function, protect data
  std::string get_name() const { return name_; }

private:
  std::string name_;
};

// IV. Multiple inheriance
class Rectangle : public Shape, public Base {
public:
  Rectangle(int l, int w) : Shape("Rectangle"), length_(l), width_(w) {
    INFO_STREAM("Construct Rectangle");
  }
  int area() const override { return length_ * width_; }
  // V. Friend class & function can access private member
  friend class RectGroup;
  friend void print_info(const Rectangle &rect);

private:
  int length_;
  int width_;
};

void print_info(const Rectangle &rect) {
  INFO_STREAM("Area of " << rect.get_name() << " is " << rect.area());
}

class RectGroup {
public:
  RectGroup() {}
  void add_rect(const Rectangle &rect) { v_rects_.push_back(rect); }
  void print_info() {
    for (const auto &rect : v_rects_) {
      INFO_STREAM("Length:" << rect.length_ << ", width: " << rect.width_);
    }
  }

private:
  std::vector<Rectangle> v_rects_;
};

int main() {
  { // Array
    print_header("Test array");
    int arr1[5] = {1, 3, 4, 5, 6};
    print_array(arr1, sizeof(arr1) / sizeof(arr1[0]));
    int arr2[] = {1, 3, 4, 5, 6, 3};
    print_array(arr2, sizeof(arr2) / sizeof(arr2[0]));
    // The array name is treated as a pointer that stored the memory address of
    // the first element of the array
    int *ptr = arr1;
    INFO_STREAM("Address of arr1: " << &arr1);
    INFO_STREAM("Address of arr1: " << ptr);
    INFO_STREAM("The first element of arr1: " << *arr1);
    INFO_STREAM("The second element of arr1: " << *(arr1 + 1));
  }
  { // Size of class
    print_header("Inspect class size");
    // Function won't increase size of class
    INFO_STREAM("Size of Base: " << sizeof(Base));                         // 12
    INFO_STREAM("Size of PublicDerived: " << sizeof(PublicDerived));       // 12
    INFO_STREAM("Size of ProtectedDerived: " << sizeof(ProtectedDerived)); // 12
    INFO_STREAM("Size of PrivateDerived: " << sizeof(PrivateDerived));     // 12
    INFO_STREAM("Size of String: " << sizeof(std::string));                // 32
    INFO_STREAM("Size of Shape: " << sizeof(Shape));                       // 40
    INFO_STREAM("Size of Rectangle: " << sizeof(Rectangle));               // 64
  }
  { // Friend class and function
    print_header("Test friend class and function");
    Rectangle rect(1, 1);
    INFO_STREAM("Friend class:");
    RectGroup groups;
    groups.add_rect(rect);
    groups.print_info();
    INFO_STREAM("Friend function:");
    print_info(rect);
  }
  return 0;
}