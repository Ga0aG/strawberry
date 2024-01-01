#include "../include/helper.hh"

int main() {
  { // const pointer
    print_header("Test const");
    int a = 2, b = 3;
    const int *ptr1 = &a;
    // *ptr1 = 5;// ERROR
    ptr1 = &b;
    int *const ptr2 = &a;
    // ptr1 = &b; // ERROR
    *ptr2 = 5;
  }
  { // new & delete operator
    print_header("Test new and delete");
    int *ptr = new int(1);
    delete ptr; // return the memory to the operating system.  ptr is now a
                // dangling pointer.
    // std::cout << *ptr; // ERROR. Dereferencing a dangling pointer will cause
    // undefined behavior.
    // delete ptr; // ERROR. trying to deallocate the memory
    // again will also lead to undefined behavior.
  }
  { // Segmentation fault
    print_header("Test segmentation fault");
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
  }
  return 0;
}