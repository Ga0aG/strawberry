#include "../../include/helper.hh"
#include <climits>
#include <functional>
#include <list>
#include <queue>
#include <unordered_map>
#include <vector>

struct Node {
  Node(int i, int c) : ind(i), cost(c) {}
  int ind;
  int cost;
};

bool operator<(const Node &lhs, const Node &rhs) {
  return lhs.cost > rhs.cost;
};

class Graph {
public:
  // Constructor
  Graph(int v) {
    V = v;
    adj.resize(v);
  }
  // Function to add an edge to graph
  void addEdge(int v, int w) { adj[v].push_back(w); }
  int V;
  std::vector<std::list<int>> adj;
};

int main() {
  { // Astar
    print_header("Test Astar");
    std::vector<std::vector<int>> distances{
        {0, 2, 3, -1}, // A - start
        {2, 0, -1, 6}, // B
        {3, -1, 0, 2}, // C
        {-1, 6, 2, 0}  // D - goal
    };

    // Plan from A to D
    std::priority_queue<Node> nodes_to_visit;
    int start = 0;
    int goal = 3;
    nodes_to_visit.push(Node(start, 0));
    std::vector<int> costs(4, INT_MAX);
    std::unordered_map<int, int> prevs;
    costs[start] = 0;
    prevs[start] = -1;

    bool found_goal = false;
    while (!nodes_to_visit.empty()) {
      Node curr = nodes_to_visit.top();
      INFO_STREAM("Visit " << curr.ind);
      if (curr.ind == goal) {
        found_goal = true;
        break;
      }
      nodes_to_visit.pop();
      for (int i = 0; i < distances[curr.ind].size(); ++i) {
        if (i == curr.ind || distances[curr.ind][i] == -1) {
          continue;
        }
        int new_cost = costs[curr.ind] + distances[curr.ind][i];
        if (costs[i] > new_cost) {
          costs[i] = new_cost;
          prevs[i] = curr.ind;
          nodes_to_visit.push(Node(i, new_cost));
        }
      }
    }
    if (found_goal) {
      INFO_STREAM("Cost: " << costs[goal]);
      int curr = goal;
      while (curr != start) {
        INFO_STREAM("Back Step: " << curr);
        curr = prevs[curr];
      }
      INFO_STREAM("Back Step: " << curr);
    }
  }
  { // Priority queue
    print_header("Priority queue");
    std::priority_queue<Node> queue;
    queue.push(Node(1, 2));
    queue.push(Node(2, 1));
    queue.push(Node(3, 5));
    while (!queue.empty()) {
      auto node = queue.top();
      queue.pop();
      INFO_STREAM("Pop " << node.ind << ", data: " << node.cost);
    }
  }
  Graph g(5);
  { // Build graph
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 0);
    g.addEdge(1, 2);
    g.addEdge(1, 4);
    g.addEdge(2, 0);
    g.addEdge(2, 1);
    g.addEdge(2, 3);
    g.addEdge(4, 1);
    //       4
    //       |
    // 0 --- 1
    //  \    |
    //   --- 2 --- 3
  }
  { // BFS
    print_header("Breadth first search");
    INFO_STREAM("Start from vertex 2");
    int s = 2;
    // Mark all the vertices as not visited
    std::vector<bool> visited;
    visited.resize(5, false);
    // Create a queue for BFS
    std::list<int> queue;
    // Mark the current node as visited and enqueue it
    visited[s] = true;
    queue.push_back(s);
    while (!queue.empty()) {
      // Dequeue a vertex from queue and print it
      s = queue.front();
      INFO_STREAM("Visit " << s);
      queue.pop_front();
      // Get all adjacent vertices of the dequeued
      // vertex s.
      // If an adjacent has not been visited,
      // then mark it visited and enqueue it
      for (auto adjacent : g.adj[s]) {
        if (!visited[adjacent]) {
          visited[adjacent] = true;
          queue.push_back(adjacent);
        }
      }
    }
  }
  { // DFS
    print_header("Depth first search");
    INFO_STREAM("Start from vertex 2");
    int s = 2;
    // Mark all the vertices as not visited
    std::vector<bool> visited;
    visited.resize(5, false);
    // Create a queue for BFS
    std::list<int> queue;
    // Mark the current node as visited and enqueue it
    visited[s] = true;
    queue.push_back(s);
    std::function<void(int curr)> dfs;
    dfs = [&](int curr) {
      for (auto adjacent : g.adj[curr]) {
        if (!visited[adjacent]) {
          visited[adjacent] = true;
          queue.push_back(adjacent);
          dfs(adjacent);
        }
      }
    };
    while (!queue.empty()) {
      // Dequeue a vertex from queue and print it
      s = queue.front();
      INFO_STREAM("Visit " << s);
      queue.pop_front();
      dfs(s);
    }
  }
  return 0;
}