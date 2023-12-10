#include "../include/node.hh"
#include <functional>
#include <queue>

int main() {
  { // deleteNode
    print_header("Delete node");
    std::shared_ptr<ListNode> head = std::make_shared<ListNode>(1);
    ;
    head->next = std::make_shared<ListNode>(2);
    ;
    INFO_STREAM("Before: " << head);
    head = deleteNode(head, 1);
    INFO_STREAM("After: " << head);
  }
  { // Clear list
    print_header("Clear list");
    std::shared_ptr<ListNode> head = std::make_shared<ListNode>(3);
    head->next = std::make_shared<ListNode>(4);
    INFO_STREAM("Before: " << head);
    clear(head);
    INFO_STREAM("After: " << head);
  }
  { // Merge multiple lists in ascending order
    print_header("Merge list");
    std::shared_ptr<ListNode> head1 = std::make_shared<ListNode>(5);
    head1->next = std::make_shared<ListNode>(9);
    head1->next->next = std::make_shared<ListNode>(10);
    std::shared_ptr<ListNode> head2 = std::make_shared<ListNode>(6);
    head2->next = std::make_shared<ListNode>(7);
    head2->next->next = std::make_shared<ListNode>(8);
    // Recursive 递归
    std::function<std::shared_ptr<ListNode>(std::shared_ptr<ListNode>,
                                            std::shared_ptr<ListNode>)>
        merge_list;
    merge_list = [&merge_list](
                     std::shared_ptr<ListNode> a,
                     std::shared_ptr<ListNode> b) -> std::shared_ptr<ListNode> {
      if (a == nullptr) {
        return b;
      } else if (b == nullptr) {
        return nullptr;
      } else {
        if (a->val <= b->val) {
          a->next = merge_list(a->next, b);
          return a;
        } else {
          b->next = merge_list(b->next, a);
          return b;
        }
      }
    };
    head1 = merge_list(head1, head2);
    INFO_STREAM("After merge: " << head1);
  }
  INFO_STREAM("Bye~");
  return 0;
}