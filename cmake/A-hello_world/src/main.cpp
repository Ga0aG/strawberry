#include "simple_math/sum.hh"
#include <iostream>

int arr[2]={1,2};
int main()
{
  int val = sum(arr,2);
  std::cout << val << std::endl;
  return 0;
}