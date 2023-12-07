#include "simple_math/sum.hh"
#include <gtest/gtest.h>

GTEST_TEST(TestSum, sum_success) {
  int arr[2] = {1, 2};
  EXPECT_EQ(3, sum(arr, 2));
}

GTEST_TEST(TestSum, sum_failure) {
  int arr[2] = {1, 2};
  EXPECT_EQ(30, sum(arr, 2));
}
