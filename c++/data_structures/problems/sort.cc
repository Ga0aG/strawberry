#include "../../include/helper.hh"
#include <vector>

using int_iterator = std::vector<int>::iterator;

void swap(int_iterator it1, int_iterator it2) {
  if (it1 == it2) {
    return;
  }
  INFO_STREAM("Swap " << *it1 << " and " << *it2);
  int data = *it1;
  *it1 = *it2;
  *it2 = data;
}

int_iterator quick_partition(int_iterator start, int_iterator end) {
  int pivot = *start;
  int count = 0;
  for (auto it = start + 1; it != end; ++it) {
    if (*it < pivot) {
      ++count;
    }
  }
  swap(start, start + count);
  int_iterator i = start;
  int_iterator ipivot = start + count;
  int_iterator j = ipivot; // j=end,--end, segmentation fault
  // if(i==ipivot)
  // {
  //   return ipivot;
  // }
  INFO_STREAM("Pivot: " << *ipivot);
  while (true) {
    while (*i < *ipivot) {
      ++i;
    }
    while (j != end && *j >= pivot) {
      ++j;
    }
    if (i < ipivot && j != end) {
      swap(i, j);
    } else {
      break;
    }
  }
  // std::vector<int> results;
  // for(auto it = start; it!=end;++it)
  // {
  //   results.push_back(*it);
  // }
  // print_vector(results);

  return ipivot;
}

void quick_sort(int_iterator start, int_iterator end) {
  if (end <= start) {
    return;
  }
  // 二叉树的前序遍历
  // 数组中选取一个数作为pivot,
  // 比pivot小的放左边，比pivot大的放右边，两边递归使用quick sort
  int_iterator p = quick_partition(start, end);
  quick_sort(start, p - 1);
  quick_sort(p + 1, end);
}

void merge_sort(int_iterator start, int_iterator end) {
  if (end - start <= 1) {
    return;
  }
  int size = end - start;
  auto mid = start + size / 2;
  // 后续遍历
  merge_sort(start, mid); //左闭右开
  merge_sort(mid, end);
  auto it1 = start;
  auto it2 = mid;
  std::vector<int> sorted;
  while (it1 != mid || it2 != end) {
    if (it1 == mid) {
      sorted.push_back(*it2);
      ++it2;
    } else if (it2 == end) {
      sorted.push_back(*it1);
      ++it1;
    } else if (*it1 <= *it2) {
      sorted.push_back(*it1);
      ++it1;
    } else {
      sorted.push_back(*it2);
      ++it2;
    }
  }
  for (unsigned int ind = 0; ind < size; ++ind) {
    *(start + ind) = sorted[ind];
  }
}

int main() {
  std::vector<int> values = {4, 2, 5, 3, 6, 1, 3, 8};
  { // Quick sort
    print_header("Quick sort");
    INFO_STREAM("Original vector: ");
    print_vector(values);
    std::vector<int> nums(values.begin(), values.end());
    quick_sort(nums.begin(), nums.end());
    print_vector(nums);
  }
  { // Merge sort
    print_header("Merge sort");
    INFO_STREAM("Original vector: ");
    print_vector(values);
    std::vector<int> nums(values.begin(), values.end());
    merge_sort(nums.begin(), nums.end());
    print_vector(nums);
  }
  { // Binary search
    print_header("Binary search");
    std::vector<int> nums(values.begin(), values.end());
    quick_sort(nums.begin(), nums.end());
    INFO_STREAM("Nums: ");
    print_vector(nums);
    INFO_STREAM("Find target 5");
    int target = 5;
    int left = 0;
    int right = nums.size() - 1;
    while (left <= right) {
      int mid = (right - left) / 2 + left;
      if (nums[mid] == target) {
        INFO_STREAM("Found target, index: " << mid);
        break;
      } else if (nums[mid] > target) {
        right = mid - 1;
      } else {
        left = mid + 1;
      }
    }
  }
  { // Bubble sort
    print_header("Bubble sort");
    INFO_STREAM("Original vector: ");
    print_vector(values);
    std::vector<int> nums(values.begin(), values.end());
    int i = 0;
    while (i < nums.size() - 1) {
      int j = 0;
      while (j < nums.size() - 1 - i) {
        // 把大的数往后推，先把最大的推到最后的位置，再把第二大的推到倒数第二的位置
        if (nums[j] > nums[j + 1]) {
          swap(nums.begin() + j, nums.begin() + j + 1);
        }
        ++j;
      }
      ++i;
    }
    print_vector(nums);
  }
  return 0;
}