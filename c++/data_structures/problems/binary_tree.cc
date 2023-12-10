#include "../../include/helper.hh"
#include <memory>

class Node {
public:
  Node(int val) {
    data = val;
    left = nullptr;
    right = nullptr;
  }
  int data;
  std::shared_ptr<Node> left;
  std::shared_ptr<Node> right;
};

void preorder_traverse(std::shared_ptr<Node> node, std::vector<int> &results) {
  if (node != nullptr) {
    results.push_back(node->data);
    preorder_traverse(node->left, results);
    preorder_traverse(node->right, results);
  }
  return;
}

void inorder_traverse(std::shared_ptr<Node> node, std::vector<int> &results) {
  if (node != nullptr) {
    inorder_traverse(node->left, results);
    results.push_back(node->data);
    inorder_traverse(node->right, results);
  }
  return;
}

void postorder_traverse(std::shared_ptr<Node> node, std::vector<int> &results) {
  if (node != nullptr) {
    postorder_traverse(node->left, results);
    postorder_traverse(node->right, results);
    results.push_back(node->data);
  }
  return;
}

std::shared_ptr<Node> buildTree(const std::vector<int> &preorder,
                                const std::vector<int> &inorder) {
  if (preorder.empty()) {
    return nullptr;
  }
  // 先序遍历第一个是根节点
  std::shared_ptr<Node> root = std::make_shared<Node>(preorder[0]);
  int ind = 0;
  while (inorder[ind] != root->data) {
    ++ind; // size of ltree
  }
  // 中序遍历根节点左边是左子树，右边是右子树
  std::vector<int> lin(inorder.begin(), inorder.begin() + ind);
  std::vector<int> rin(inorder.begin() + ind + 1, inorder.end());
  std::vector<int> lpre(preorder.begin() + 1, preorder.begin() + 1 + ind);
  std::vector<int> rpre(preorder.begin() + 1 + ind, preorder.end());
  root->left = buildTree(lpre, lin);
  root->right = buildTree(rpre, rin);
  return root;
}

int main() {
  //    8
  //    /\
  //  10  11
  //  /\   \
  // 2 15   5
  std::shared_ptr<Node> root = std::make_shared<Node>(8);
  root->left = std::make_shared<Node>(10);
  root->right = std::make_shared<Node>(11);
  root->left->left = std::make_shared<Node>(2);
  root->left->right = std::make_shared<Node>(15);
  root->right->right = std::make_shared<Node>(5);
  { // traverse
    print_header("Test traverse");
    std::vector<int> results;
    INFO_STREAM("Preorder: ");
    preorder_traverse(root, results);
    print_vector(results); // 8, 10, 2, 15, 11, 5,
    results.clear();
    INFO_STREAM("Inorder: ");
    inorder_traverse(root, results); // 2, 10, 15, 8, 11, 5,
    print_vector(results);
    results.clear();
    INFO_STREAM("Postorder: ");
    postorder_traverse(root, results); // 2, 15, 10, 5, 11, 8,
    print_vector(results);
  }
  { // 105. construct binary tree from preorder and inorder traverse
    print_header("Rebuild tree");
    std::vector<int> preorder;
    std::vector<int> inorder;
    preorder_traverse(root, preorder);
    inorder_traverse(root, inorder);
    auto node = buildTree(preorder, inorder);
    std::vector<int> results;
    INFO_STREAM("Postorder: ");
    postorder_traverse(node, results);
    print_vector(results);
  }
  return 0;
}