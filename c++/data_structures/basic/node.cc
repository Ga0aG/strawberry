#include "../include/node.hh"

std::shared_ptr<ListNode> deleteNode(std::shared_ptr<ListNode> root, int val) {
  std::shared_ptr<ListNode> curr = root->next;
  std::shared_ptr<ListNode> pre = root;
  if (pre->val == val) {
    return curr;
  }
  while (curr != nullptr && curr->val != val) {
    pre = curr;
    curr = curr->next;
  }
  if (curr != nullptr) {
    pre->next = curr->next;
  }
  return root;
}

std::ostream &operator<<(std::ostream &os, std::shared_ptr<ListNode> root) {
  std::shared_ptr<ListNode> curr = root;
  while (curr != nullptr) {
    os << curr->val << ", ";
    curr = curr->next;
  }
  return os;
}

void clear(std::shared_ptr<ListNode> &root) { root = nullptr; }
