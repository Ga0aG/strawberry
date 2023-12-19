#include "../include/helper.hh"

int main()
{
  {// lvalue reference
    print_header("lvalue reference");
    int a  = 8;
    INFO_STREAM("Before, A: "<<a);
    int &ref = a;
    ref = 7;
    INFO_STREAM("After, A: " << a);
  }
  return 0;
}