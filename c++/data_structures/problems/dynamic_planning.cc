#include "../../include/helper.hh"
#include <functional>

using namespace std;
int main() {
  { // Fibonacci
    print_header("Test Fibonacci");
    // 暴力解法
    function<int(int)> fib1;
    fib1 = [&fib1](int n) {
      if (n == 1 || n == 2) {
        return 1;
      }
      return fib1(n - 1) + fib1(n - 2);
    };
    INFO_STREAM("Fibonacci of 6 is: " << fib1(6));
    // 备忘录
    std::vector<int> note(7, 0);
    function<int(int)> fib2;
    fib2 = [&fib2, &note](int n) {
      if (n == 1 || n == 2) {
        return 1;
      } else if (note[n - 1] != 0) {
        return note[n - 1];
      } else {
        note[n - 1] = fib2(n - 1) + fib2(n - 2);
      }
      return note[n - 1];
    };
    INFO_STREAM("Fibonacci of 7 is: " << fib2(7));
    // 自下而上
    function<int(int)> fib3;
    fib3 = [&fib3](int n) {
      if (n == 0 || n == 1) {
        return 1;
      }
      int prev = 1;
      int curr = 1;
      for (int i = 3; i <= n; ++i) {
        int sum = prev + curr;
        prev = curr;
        curr = sum;
      }
      return curr;
    };
    INFO_STREAM("Fibonacci of 8 is: " << fib3(8));
  }
}
