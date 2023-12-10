#include "../../include/helper.hh"

int main() {
  { // 223. Rectangle overlap
    print_header("Compute rectangle overlap");
    int ax1 = -3, ay1 = 0, ax2 = 3, ay2 = 4, bx1 = 0, by1 = -1, bx2 = 9,
        by2 = 2;
    INFO_STREAM("ax1 = -3, ay1 = 0, ax2 = 3, ay2 = 4");
    INFO_STREAM("bx1 = 0, by1 = -1, bx2 = 9, by2 = 2");
    int o_h = ay2 - ay1 + by2 - by1 - (std::max(ay2, by2) - std::min(ay1, by1));
    int o_w = ax2 - ax1 + bx2 - bx1 - (std::max(ax2, bx2) - std::min(ax1, bx1));
    int area = (ay2 - ay1) * (ax2 - ax1) + (by2 - by1) * (bx2 - bx1);
    if (o_h <= 0 || o_w <= 0) {
      INFO_STREAM("Overlap area: 0");
    } else {
      INFO_STREAM("Overlap area: " << o_h * o_w);
    }
  }
  { // area of polygon
    print_header("Area of polygon");
    int a[4][2] = {{1, 1}, {1, 3}, {3, 3}, {3, 1}};
    float area = 0;
    int i = 0;
    for (; i < 3; ++i) {
      area += (a[i][0] * a[i + 1][1] - a[i][1] * a[i + 1][0]) / 2.0;
    }
    area += (a[i][0] * a[0][1] - a[i][1] * a[0][0]) / 2.0;
    INFO_STREAM("Expect 4, get: " << std::abs(area));
  }
  return 0;
}