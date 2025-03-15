#include "../include/helper.hh"
#include <memory>

template <class T>
class UseResource{
public:
std::shared_ptr<T> ptr;
void Show(){
  if (ptr){ptr->ShowResource();}
}
};

class Resource : public std::enable_shared_from_this<Resource>
{
public:
  Resource() { INFO_STREAM("Resource acquired"); }
  ~Resource() { INFO_STREAM("Resource destroyed"); }
  void ShareResource(UseResource<Resource>& someone) {someone.ptr = shared_from_this();}
  void ShowResource() { INFO_STREAM("Show Resource"); }
};
std::ostream &operator<<(std::ostream &out, const Resource &)
{
  out << "I am a resource";
  return out;
}

template <class T>
class Auto_ptr1
{
  T *m_ptr;

public:
  // Pass in a pointer to "own" via the constructor
  Auto_ptr1(T *ptr = nullptr)
      : m_ptr(ptr)
  {
  }

  // The destructor will make sure it gets deallocated
  ~Auto_ptr1()
  {
    delete m_ptr;
  }

  // Overload dereference and operator-> so we can use Auto_ptr1 like m_ptr.
  T &operator*() const { return *m_ptr; }
  T *operator->() const { return m_ptr; }
};

template <class T>
class Auto_ptr2
{
  T *m_ptr;

public:
  // Pass in a pointer to "own" via the constructor
  Auto_ptr2(T *ptr = nullptr)
      : m_ptr(ptr)
  {
  }

  // The destructor will make sure it gets deallocated
  ~Auto_ptr2()
  {
    delete m_ptr;
  }

  // Do deep copy of a.m_ptr to m_ptr
  Auto_ptr2(const Auto_ptr2 &a)
  {
    INFO_STREAM("Copy constructor is called, do a deep copy");
    m_ptr = new T;
    *m_ptr = *a.m_ptr;
  }

  // Copy assignment
  // Do deep copy of a.m_ptr to m_ptr
  Auto_ptr2 &operator=(const Auto_ptr2 &a)
  {
    INFO_STREAM("Copy assignment is called, do a deep copy");
    // Self-assignment detection
    if (&a == this)
      return *this;
    // Release any resource we're holding
    delete m_ptr;
    // Copy the resource
    m_ptr = new T;
    *m_ptr = *a.m_ptr;
    return *this;
  }

  bool isNull() const { return m_ptr == nullptr; }

  // Overload dereference and operator-> so we can use Auto_ptr2 like m_ptr.
  T &operator*() const { return *m_ptr; }
  T *operator->() const { return m_ptr; }
};

template <class T>
class Auto_ptr3
{
  T *m_ptr;

public:
  // Pass in a pointer to "own" via the constructor
  Auto_ptr3(T *ptr = nullptr)
      : m_ptr(ptr)
  {
  }

  // The destructor will make sure it gets deallocated
  ~Auto_ptr3()
  {
    delete m_ptr;
  }

  // Copy constructor -- no copying allowed!
  Auto_ptr3(const Auto_ptr3 &a) = delete;

  // Move constructor
  // Transfer ownership of a.m_ptr to m_ptr
  Auto_ptr3(Auto_ptr3 &&a)
      : m_ptr(a.m_ptr)
  {
    INFO_STREAM("Move constructor is called");
    a.m_ptr = nullptr;
  }

  // Copy assignment -- no copying allowed!
  Auto_ptr3 &operator=(const Auto_ptr3 &a) = delete;

  // Move assignment
  // Transfer ownership of a.m_ptr to m_ptr
  Auto_ptr3 &operator=(Auto_ptr3 &&a)
  {
    INFO_STREAM("Move assignment is called, do a deep copy");
    // Self-assignment detection
    if (&a == this)
      return *this;

    // Release any resource we're holding
    delete m_ptr;

    // Transfer ownership of a.m_ptr to m_ptr
    m_ptr = a.m_ptr;
    a.m_ptr = nullptr;

    return *this;
  }

  bool isNull() const { return m_ptr == nullptr; }

  // Overload dereference and operator-> so we can use Auto_ptr3 like m_ptr.
  T &operator*() const { return *m_ptr; }
  T *operator->() const { return m_ptr; }
};

template <template <typename> class T, typename P>
T<P> generateResource()
{
  T<P> res(new P);
  return res;
}

void AccessResource(std::unique_ptr<Resource> &ptr)
{
  INFO_STREAM("" << *ptr);
}

template <class T>
class CircularDepA
{
public:
  std::shared_ptr<T> b;
  ~CircularDepA() { INFO_STREAM("ByeBye, CircularDepA"); }
};

class CircularDepB
{
public:
  std::shared_ptr<CircularDepA<CircularDepB>> a;
  ~CircularDepB() { INFO_STREAM("ByeBye, CircularDepB"); }
};

class CircularDepWeakB
{
public:
  std::weak_ptr<CircularDepA<CircularDepWeakB>> a;
  ~CircularDepWeakB() { INFO_STREAM("ByeBye, CircularDepWeakB"); }
};

int main()
{
  bool runAll = true;
  if (false || runAll) // basic pointer
  {
    // A pointer is a variable that holds a memory address as its value.
    print_header("pointer");
    int x = 5;
    int *p = &x;
    // vs array
    int arr[] = {1, 2, 3};
    int *ptr = arr;
    INFO_STREAM("Compare arr with ptr");
    INFO_STREAM("Size: " << sizeof(arr) << ", " << sizeof(ptr)); // 4*3,
    INFO_STREAM("Access 2nd element: " << arr[1] << ", " << ptr[1]);
  }
  if (false || runAll) // new & delete operator
  {
    print_header("new and delete");
    int *ptr = new int(1);
    delete ptr; // return the memory to the operating system.  ptr is now a dangling pointer.
  }
  if (false || runAll) // const pointer
  {
    print_header("with const");
    int a = 2, b = 3;
    const int *ptr1 = &a; // const data
    // *ptr1 = 5;// ERROR
    ptr1 = &b;
    int *const ptr2 = &a; // const pointer
    // ptr1 = &b; // ERROR
    *ptr2 = 5;
  }
  if (false || runAll) // Segmentation fault
  {
    print_header("segmentation fault");
    // A. Dereferencing uninitialized pointer
    // int *ptr = nullptr;
    // int i = 1;
    // *ptr = 1;
    // 空指针访问即尝试操作地址为0的内存区域，由于该区域内存是禁止访问的区域，所以当发生空指针访问时进程就会收到SIGSEGV信号发生

    // B. Access out of array index bounds
    // int arr[2];
    // arr[3] = 0;

    // C. Accessing an address that is freed
    // int *ptr = new int(1);
    // delete ptr;
    // INFO_STREAM("Value: "<<*ptr);
    // 它可能指向一块不存在的内存页，也可能是指向一块没有访问权限的内存区域，如果是这样你应该感谢segmentation
    // fault段错误，因为问题很快就会暴露出来而不会被蔓延，否则如果指向了一块合法内存，那对内存的破坏将会有无法预测的事情发生，
    // 可能只是纂改了你的数据，也可能是破坏了内存结构，这个时候错误可能被蔓延到一个无法预测的时刻。
    // Dereferencing a dangling pointer will cause undefined behavior.

    // D. Delete ptr twice
    // delete ptr; // ERROR. trying to deallocate the memory again will also lead to undefined behavior
  }
  if (false || runAll) // manual smart pointer
  {
    print_header("manual smart pointer");
    {                                         // auto deconstruct
      Auto_ptr1<Resource> res1(new Resource); // Note the allocation of memory here but no explicit delete needed
      // > Auto_ptr1<Resource> res2(res1);
      /**
       * * Because we haven’t supplied a copy constructor or an assignment operator, C++ provides one for us. And the functions it provides do **shallow copies**. So when we initialize res2 with res1, both Auto_ptr1 variables are pointed at the same Resource. When res2 goes out of the scope, it deletes the resource, leaving res1 with a **dangling pointer**. When res1 goes to delete its (already deleted) Resource, crash!
       */
    } // res1 goes out of scope here, and destroys the allocated Resource for us
    { // Add copy constructor & assignment
      INFO_STREAM(">>>>>>><<<<<<<<");
      Auto_ptr2<Resource> res1;
      res1 = generateResource<Auto_ptr2, Resource>();
      /**
        [1741876586.367432] Resource acquired
        [1741876586.367440] Copy assignment is called, do a deep copy
        [1741876586.367446] Resource acquired
        [1741876586.367452] Resource destroyed
        [1741876586.367457] Resource destroyed
       */
      // * a lot of resource creation and destruction going on in a simple program
    }
    { // Add move constructor & assignment, disable copy constructor and assignment
      INFO_STREAM(">>>>>>><<<<<<<<");
      Auto_ptr3<Resource> res1;
      res1 = generateResource<Auto_ptr3, Resource>();
      // Move assignment is called, do a deep copy
    }
  }
  if (false || runAll) // STL smart pointer
  {
    print_header("STL smart pointer");
    { // unique_ptr
      // 自动析构，不支持复制和赋值，一定程度避免了一些误操作导致指针所有权转移
      std::unique_ptr<Resource> p1(new Resource());
      std::unique_ptr<Resource> p2 = std::make_unique<Resource>();
      // std::unique_ptr<Resource> p3 = p1; // ERROR
      std::unique_ptr<Resource> p3 = std::move(p1);
      INFO_STREAM("p1 is " << (p1 ? "not null" : "null")); // null
      INFO_STREAM("p3 is " << (p3 ? "not null" : "null")); // not null
      if (p2)
      {
        INFO_STREAM("" << *p2);
      }
      p2.reset(new Resource());
      /* [1741882082.753086] Resource acquired
         [1741882082.753092] Resource destroyed */
      AccessResource(p2);
    }
    { // shared_ptr
      INFO_STREAM(">>>>>>><<<<<<<<");
      // 使用引用计数，实现对同一块内存可以有多个引用，在最后一个引用被释放时，指向的内存才释放
      std::shared_ptr<Resource> ptr1 = std::make_shared<Resource>();
      std::shared_ptr<Resource> ptr2 = ptr1;
      INFO_STREAM("use count: " << ptr1.use_count());
      ptr1.reset(); // deletes managed object
      ptr2.reset();
      // Resource destroyed
      // > 循环引用
      std::shared_ptr<CircularDepA<CircularDepB>> spa = std::make_shared<CircularDepA<CircularDepB>>();
      std::shared_ptr<CircularDepB> spb = std::make_shared<CircularDepB>();
      spa->b = spb;                                      // spb: use_count = 2
      INFO_STREAM("spb use count: " << spb.use_count()); // 2
      // spb->a = spa;
      // 退出作用域后只会让spa/spb的引用记数减一，结果还是1,不等于0,不会自动析构
    }
    { // weak_ptr
      INFO_STREAM(">>>>>>><<<<<<<<");
      std::shared_ptr<CircularDepA<CircularDepWeakB>> spa = std::make_shared<CircularDepA<CircularDepWeakB>>();
      std::shared_ptr<CircularDepWeakB> spb = std::make_shared<CircularDepWeakB>();
      spb->a = spa;
      spa->b = spb;
      INFO_STREAM("spa use count: " << spa.use_count()); // 1, 强引用计数
      INFO_STREAM("spb use count: " << spb.use_count()); // 2
      /* [1742040787.790077] ByeBye, CircularDepA
        [1742040787.790084] ByeBye, CircularDepWeakB */
    }
    { // shared_from_this
      INFO_STREAM(">>>>>>><<<<<<<<");
      // * This is particularly useful when you want to ensure that multiple shared pointers manage the same object without creating separate ownership groups.
      UseResource<Resource> someone = UseResource<Resource>();
      std::shared_ptr<Resource> ptr = std::make_shared<Resource>();
      ptr->ShareResource(someone);
      ptr.reset();
      INFO_STREAM("Use count: " << ptr.use_count()); // 0
      someone.Show();
      INFO_STREAM("Use count: " << someone.ptr.use_count()); // 1
      // Resource destroyed
    }
  }
  return 0;
}