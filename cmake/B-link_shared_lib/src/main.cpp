#include <iostream>
#include <simple_math/simple_math/sum.hh>

int arr[2] = {1, 2};
int main() {
  int val = sum(arr, 2);
  std::cout << val << std::endl;
  return 0;
}