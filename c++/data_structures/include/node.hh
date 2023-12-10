#pragma once
#include "../../include/helper.hh"

#include <iostream>
#include <memory>

struct ListNode {
  int val;
  std::shared_ptr<ListNode> next;
  ListNode(int v) : val(v), next(nullptr) {}
  ~ListNode() { INFO_STREAM("Node " << val << " is deconstructed"); }
};

std::shared_ptr<ListNode> deleteNode(std::shared_ptr<ListNode> root, int val);
std::shared_ptr<ListNode> addNode(std::shared_ptr<ListNode> root, int val);
std::shared_ptr<ListNode> getNode(std::shared_ptr<ListNode> root, int val);
void clear(std::shared_ptr<ListNode> &root);
std::ostream &operator<<(std::ostream &os, std::shared_ptr<ListNode> root);
// bool operator<(std::shared_ptr<ListNode> n1, std::shared_ptr<ListNode> n2) {
//   return n1->val < n2->val;
// };

struct TreeNode {
  int val;
  std::shared_ptr<ListNode> left;
  std::shared_ptr<ListNode> right;
  TreeNode(int v) : val(v), left(nullptr), right(nullptr) {}
};
