#include "../include/helper.hh"
#include <algorithm>

int main() {
  { // sort
    print_header("Test sort");
    int arr[] = {32, 71, 12, 45, 26, 80, 53, 33};
    std::vector<int> vec(arr, arr + 8);
    INFO_STREAM("Original vector: ");
    print_vector(vec);
    INFO_STREAM("Sort in ascending order: ");
    std::sort(vec.begin(), vec.end());
    print_vector(vec);
    INFO_STREAM("Sort in descending order: ");
    std::sort(vec.begin(), vec.end(), [](int a, int b) { return a > b; });
    print_vector(vec);
    INFO_STREAM("Sort with compare object: ");
    struct cmp {
      bool operator()(int a, int b) { return a > b; }
    } cmpobject;
    std::sort(vec.begin(), vec.end(), cmpobject);
    print_vector(vec);
  }
  { // Non-modifying sequence operations
    print_header("Test Non-modifying sequence operations");
    int arr[] = {32, 71, 12, 45, 26, 80, 53, 33};
    std::vector<int> vec(arr, arr + 8);
    INFO_STREAM("Original vector: ");
    print_vector(vec);
    auto it = std::find(vec.begin(), vec.end(), 71); // find_if
    INFO_STREAM("Search 71 in vec:");
    if (it == vec.end()) {
      INFO_STREAM("Failed to find 71 in vec");
    } else {
      INFO_STREAM("Found 71 in vec");
    }
    INFO_STREAM("Doule element in vec:");
    std::for_each(vec.begin(), vec.end(), [&](int &x) { x *= 2; });
    print_vector(vec);
    INFO_STREAM("Count value bigger than 40 in vec: ");
    auto num =
        std::count_if(vec.begin(), vec.end(), [](int x) { return x > 40; });
    INFO_STREAM("Total num: " << num);
  }
  { // Modifying sequence operations
    print_header("Test modifying sequence operations");
    int arr[] = {32, 71, 12, 45, 26, 80, 53, 33};
    std::vector<int> vec(arr, arr + 8);
    INFO_STREAM("Reverse vec:");
    std::reverse(vec.begin(), vec.end());
    print_vector(vec);
    INFO_STREAM("Transform vec:");
    std::vector<int> b_vec;
    b_vec.resize(vec.size());
    std::transform(vec.begin(), vec.end(), b_vec.begin(),
                   [](int x) { return x * 2; });
    print_vector(b_vec);
  }
  return 0;
}