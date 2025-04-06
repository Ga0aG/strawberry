#include "../include/helper.hh"
#include <cstring>
#include <string>
#include <cstddef>

// I. Access modifiers
class Base
{
public:
  int x_ = 0;
  Base(int val) : x_(val) {}
  void show() { INFO_STREAM("Show time: x = " << x_ << ", y = " << y_ << ", z = " << z_); }
  virtual ~Base(){INFO_STREAM("Bye, Base");}

  // protected member can be accessed by derived class
protected:
  int y_ = 0;

  // private member can only accessed by Bass class itself and friend class
private:
  int z_ = 0;
};
class PublicDerived : public Base
{
public:
  PublicDerived(int val) : Base(val) {}
  ~PublicDerived() override {INFO_STREAM("Bye, PublicDerived");}
  // x is public
  // y is protected
  // z is not accessible from PublicDerived
};
class ProtectedDerived : protected Base
{
  // x is protected
  // y is protected
  // z is not accessible from ProtectedDerived
};
// Private is the default access specifier
class PrivateDerived : private Base
{
  // x is private
  // y is private
  // z is not accessible from PrivateDerived
};

class DataAlignment1
{
public:
  char a;
  int b;
};
class DataAlignment2
{
public:
  int b;
  std::string c;
};
class DataAlignment3
{
public:
  char a;
  std::string c;
};
class DataAlignment4
{
public:
  char a;
  int b;
  std::string c;
};
class DataAlignment5
{
private:
  DataAlignment1 d;
  std::string e;
};
class DataAlignment6
{
private:
  DataAlignment2 d;
  char a;
};
class DataAlignment7
{
public:
  char a;
  std::string c;
  int b;
};

class Shape
{
public:
  // II. Constructors
  // Constructor with argument
  Shape(const std::string &name) : name_(name)
  {
    INFO_STREAM("Init shape: " << name_);
  }
  // Copy constructor
  Shape(const Shape &shape)
  {
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
class Rectangle : public Shape, public Base
{
public:
  Rectangle(int l, int w) : Shape("Rectangle"), Base(1), length_(l), width_(w)
  {
    INFO_STREAM("Construct Rectangle");
  }
  int area() const override { return length_ * width_; }
  // V. Friend class & function can access private member, readonly
  friend class RectGroup;
  friend void print_info(const Rectangle &rect);

private:
  int length_;
  int width_;
};

// Friend function
void print_info(const Rectangle &rect)
{
  INFO_STREAM("Area of " << rect.get_name() << " is " << rect.area());
}

// Friend class
class RectGroup
{
public:
  RectGroup() {}
  void add_rect(const Rectangle &rect) { v_rects_.push_back(rect); }
  void print_info()
  {
    for (const auto &rect : v_rects_)
    {
      INFO_STREAM("Length:" << rect.length_ << ", width: " << rect.width_);
    }
  }

private:
  std::vector<Rectangle> v_rects_;
};

class TestConstructor
{
public:
  int val;
  TestConstructor(int x) : val(x)
  { //构造函数
    INFO_STREAM("constructor with argument, val: " << val);
  }
  TestConstructor(const TestConstructor &ex) // 加个引用避免循环构造
  {                                          //拷贝构造函数
    val = ex.val;
    INFO_STREAM("copy constructor, val: " << val);
  }
  TestConstructor(TestConstructor &&ex) // 不要加const
  { //移动构造函数
    val = ex.val;// 通常还需要把ex的资源给free掉
    INFO_STREAM("move constructor, val: " << val);
  }
  TestConstructor &operator=(const TestConstructor &ex)
  { //赋值运算符重载
    INFO_STREAM("copy assignment, val: " << val);
    val = ex.val;
    return *this;
  }
  TestConstructor &operator=(TestConstructor &&ex)
  { //赋值运算符重载
    INFO_STREAM("move assignment, val: " << val);
    val = ex.val;
    return *this;
  }
};
void TestConstructorFun(TestConstructor a)
{
  INFO_STREAM("Access val: " << a.val);
}
void TestConstructorFunWithReference(const TestConstructor &a) // // 避免拷贝
{
  INFO_STREAM("Access val: " << a.val);
}

template <class T>
class QueueNestedClass{
  public:
  void enqueue(const T& val);
  void clear();
  QueueNestedClass<T>() = default;
  private:
  class NestedNode{
    public:
     T value_;
     NestedNode* next_;
     NestedNode(const T& val): value_(val), next_(nullptr){INFO_STREAM("Create a new nestedNode: " << value_);}
     ~NestedNode(){INFO_STREAM("Byebye nestedNode: " << value_);}
  };
  NestedNode* front = nullptr;
  NestedNode* rear = nullptr;
};
template <class T>
void QueueNestedClass<T>::enqueue(const T& val){
  NestedNode* newNode = new NestedNode(val);
  if(front==nullptr){
    front = newNode;
  }
  else{
    rear->next_ = newNode;
  }
  rear = newNode;
}
template <class T>
void QueueNestedClass<T>::clear()
{
  while(front != nullptr)
  {
    // rear = front;
    // delete front;
    // front = rear->next_; // rear指向的数据已经被删除了
    rear = front; // 保存当前的 front
    front = front->next_;     // 移动 front 到下一个节点
    delete rear;              // 删除之前的 front
  }
  // rear = nullptr;
}

int main()
{
  bool runAll = true;
  if (false || runAll) // enum
  {
    print_header("Enum");
    enum Color { red, green, blue };
    Color r = red;
    switch(r)
    {
        case red  : INFO_STREAM("red");   break;
        case green: INFO_STREAM("green"); break;
        case blue : INFO_STREAM("blue");  break;
    }
  }
  if (false || runAll) // Array
  {
    print_header("Test array");
    int arr1[5] = {1, 3, 4, 5, 6};
    int arr2[] = {1, 3, 4, 5, 6, 3};
    print_array(arr1, sizeof(arr1) / sizeof(arr1[0]));
    print_array(arr2, sizeof(arr2) / sizeof(arr2[0]));

    /* Pointer
    The array name is treated as a pointer that stored the memory address of the first element of the array */
    int *ptr = arr1;
    INFO_STREAM("Address of arr1: " << &arr1);
    INFO_STREAM("Address of arr1: " << ptr);
    INFO_STREAM("The first element of arr1: " << *arr1);
    INFO_STREAM("The second element of arr1: " << *(arr1 + 1));

    /* Char Array */
    char charArray[] = "HelloWorld";
    INFO_STREAM("CharArray: " << charArray);
    INFO_STREAM("Sizeof CharArray: " << sizeof(charArray) / sizeof(char)); // 11
    INFO_STREAM("First element of CharArray: " << *charArray);
    char dest[10];
    std::strcpy(dest, "012345678"); //"0123456789"越界，它结尾还有个\0
    INFO_STREAM("strcpy: " << dest);

    /* Multidimension array */
    int a[2][2] = {1, 2, 3, 4};
    int b[2][2] = {{1, 2}, {3, 4}};
  }
  if (false || runAll) // Size of class
  {
    print_header("Inspect class size");
    int arr[3] = {1, 2, 3};
    // Function won't increase size of class
    INFO_STREAM("Size of Base: " << sizeof(Base));                              // 12
    INFO_STREAM("Size of PublicDerived: " << sizeof(PublicDerived));            // 12
    INFO_STREAM("Size of ProtectedDerived: " << sizeof(ProtectedDerived));      // 12
    INFO_STREAM("Size of PrivateDerived: " << sizeof(PrivateDerived));          // 12
    INFO_STREAM("Size of char: " << sizeof(char));                              // 1
    INFO_STREAM("Size of String: " << sizeof(std::string));                     // 32
    INFO_STREAM("Size of arr[3]: " << sizeof(arr));                             // 12
    INFO_STREAM("Size of DataAlignment(char+int): " << sizeof(DataAlignment1)); // 8
    // string也是一个类，它的对齐单元是8字节
    INFO_STREAM("Size of DataAlignment(string+int): " << sizeof(DataAlignment2));            // 40
    INFO_STREAM("Size of DataAlignment(char+string): " << sizeof(DataAlignment3));           // 40
    INFO_STREAM("Size of DataAlignment(char+int+string): " << sizeof(DataAlignment4));       // 40
    INFO_STREAM("Size of DataAlignment(char+string+int): " << sizeof(DataAlignment7));       // 48
    INFO_STREAM("Size of DataAlignment(DataAlignment1+string): " << sizeof(DataAlignment5)); // 40
    INFO_STREAM("Size of DataAlignment(DataAlignment2+char): " << sizeof(DataAlignment6));   // 48
    INFO_STREAM("Size of Shape: " << sizeof(Shape));                                         // 40
    INFO_STREAM("Size of Rectangle: " << sizeof(Rectangle));                                 // 64
    INFO_STREAM("Member Offset of DataAlignment(char+int+string):" << offsetof(DataAlignment4, a) << ", " << offsetof(DataAlignment4, b) << ", " << offsetof(DataAlignment4, c) << ", "); // 0, 4, 8
    INFO_STREAM("Member Offset of DataAlignment(char+string+int):" << offsetof(DataAlignment7, a) << ", " << offsetof(DataAlignment7, c) << ", " << offsetof(DataAlignment7, b) << ", ");// 0, 8, 40
  }
  if (false || runAll) // Friend class and function
  {
    print_header("Test friend class and function");
    Rectangle rect(1, 1);
    INFO_STREAM("Friend class:");
    RectGroup groups;
    groups.add_rect(rect);
    groups.print_info(); // read-only
    INFO_STREAM("Friend function:");
    print_info(rect); // read-only
  }
  if (false || runAll) // Inherit
  {
    print_header("inherit");
    // Base a = Base(1);
    PublicDerived b = PublicDerived(1);
    // > 如果Base的析构没有加virtual的话，PublicDerived的析构函数就不会被调用
    // 尽管base的类型是Base*，它实际上指向的是一个PublicDerived对象, 因此它的vptr指向PublicDerived的虚函数表。如果Base的析构函数没有被声明为虚的，C++运行时会根据base指针的类型（即Base*）查找Base的析构函数。
    Base *base = new PublicDerived(1);
    delete base;
  }
  if (false || runAll) // lvalue & rvalue
  {
    print_header("lvalue & rvalue");
    int a = 1;
    int& b = a; // lvalue
    int&& c = 1; // rvalue
  }
  if (false || runAll) // constructor & assignment
  {
    print_header("constructor & assignment");
    TestConstructor a(1);
    TestConstructor b(a);
    // copy constructor, val: 1
    TestConstructor c = a;
    // copy constructor, val: 1
    TestConstructorFun(a);
    // copy constructor, val: 1
    // Access val: 1
    TestConstructorFunWithReference(a);
    // Access val: 1
    TestConstructor d(std::move(b));
    // move constructor, val: 1
    TestConstructor e(2);
    e = std::move(c);
    // move assignment, val: 2
  }
  if (false || runAll) // nested class
  {
    print_header("nested class");
    // QueueNestedClass<int> q = QueueNestedClass<int>();
    QueueNestedClass<int> q{};
    q.enqueue(1);
    q.enqueue(2);
    INFO_STREAM("Ready to clear");
    q.clear();
  }
  return 0;
}