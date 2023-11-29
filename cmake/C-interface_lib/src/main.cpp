#include "happy_math/add.hh"
#include <iostream>

int main()
{
  int val = add(1,2);
  std::cout << val << std::endl;
  int arr[2]={1,2};
  val = sum(arr,2);
  return 0;
}