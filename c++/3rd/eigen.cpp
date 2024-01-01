#include <eigen3/Eigen/Dense>
#include <iostream>

using Eigen::MatrixXd;
using namespace std;

int main() {
  Eigen::AngleAxis<double> rotation_roll(M_PI / 4, Eigen::Vector3d(0, 0, -1));
  auto rotated_x_axis = rotation_roll * Eigen::Vector3d(1, 1, -1);
  cout << rotated_x_axis << endl;
}