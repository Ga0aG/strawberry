#include "../../include/helper.hh"
#include <algorithm>
#include <array>
#include <functional>
#include <map>
#include <utility>

int main() {
  { // sliding window, maximum sum of a fixed continuous sequence
    print_header(
        "Sliding window - Find maximu sum of fixed continuous sequence");
    std::array<int, 16> nums = {1, 4, 3, 2, 5, 6, 8, 4, 2, 4, 5, 6, 9, 2, 1, 4};
    int k = 3;
    print_array(nums);
    int maxi_sum = 0;
    int sum = 0;
    for (int i = 0; i < k; ++i) {
      sum += nums[i];
    }
    maxi_sum = sum;
    for (int j = k; j < nums.size(); ++j) {
      sum -= nums[j - k];
      sum += nums[j];
      maxi_sum = sum > maxi_sum ? sum : maxi_sum;
    }
    INFO_STREAM("Maximum sum of fixed size sequence: " << maxi_sum);
  }
  { // 76. sliding window, Find shortest substring cover target characters
    print_header(
        "Sliding window - Find shortest substring cover target characters");
    std::string arr = "ADOBECODEBANCDD";
    std::string target = "ABC";
    std::string result = arr + "NO_FOUND";
    INFO_STREAM("Array: " << arr);
    int l = 0, r = 0;
    std::map<char, int> found_chars;
    for (auto c : target) {
      found_chars[c] = 0;
    }

    for (; l < arr.size() - target.size(); ++l) {
      // Move window to right until found one target character
      if (found_chars.count(arr[l]) == 0) {
        continue;
      }
      INFO_STREAM("CUR: " << arr.substr(l, r - l + 1));
      // Extend window until all characters are found
      for (; r < arr.size(); ++r) {
        if (!found_chars.count(arr[r])) {
          continue;
        }
        found_chars[arr[r]] += 1;
        INFO_STREAM("ADD: " << arr.substr(l, std::max(r - l + 1, 1)));
        print_map(found_chars);

        auto it = std::find_if(
            found_chars.begin(), found_chars.end(),
            [](const std::pair<char, bool> &pair) { return pair.second == 0; });
        bool found_solution = it == found_chars.end();
        // Update solution
        if (found_solution && result.size() > r - l + 1) {
          result = arr.substr(l, r - l + 1);
        }
        // Move window to the right
        if (found_solution || (arr[l] == arr[r] and l != r)) {
          found_chars[arr[l]] -= 1;
          ++r;
          break;
        }
      }
    }
    if (result.size() > arr.size()) {
      INFO_STREAM("RESULT: ");
    } else {
      INFO_STREAM("RESULT: " << result);
    }
  }
  { // 881. two pointers
    print_header("Two pointers");
    int limit = 5;
    std::vector<int> people{3, 5, 3, 4};
    INFO_STREAM("Limit: " << limit << ", People: ");
    print_vector(people);
    std::sort(people.begin(), people.end());
    int l = 0, r = people.size() - 1;
    int nums = 0;
    while (l < r) {
      if (people[r] + people[l] <= limit) {
        ++l;
        --r;
        ++nums;
      } else {
        --r;
        ++nums;
      }
    }
    if (l == r) {
      ++nums;
    }
    INFO_STREAM("Minimum nums of boats: " << nums);
  }
}