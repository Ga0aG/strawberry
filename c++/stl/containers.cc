#include <algorithm>
#include <deque>
#include <list>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include "../include/helper.hh"

int main()
{
    bool runAll = true;
    // sequence containers
    if (false || runAll) // array
    {
        print_header("Array");
        // fixed size
        std::array<int, 3> a1{1, 2, 3};
        INFO_STREAM("arr[1]: " << a1[1]);
    }
    if (false || runAll) // vector
    {
        print_header("Vector");
        // 动态内存，连续空间，元素访问效率高，除尾部外增删元素效率差一些
        std::vector<int> first;
        std::vector<int> second(4, 100); // {100,100,100,100}
        std::vector<int> third(second.begin(), second.end());
        std::vector<int> fourth(third);
        int myints[] = {16, 2, 77, 29};
        std::vector<int> fifth(myints, myints + sizeof(myints) / sizeof(int));
        fifth.emplace_back(10); //  arguments to forward to the constructor of the element
        std::vector<int>::iterator a = fifth.begin();
        fifth.emplace(a + 2, 100);
        fifth.insert(a + 3, 20);
        print_vector(fifth); // 16, 2, 100, 20, 77, 29, 10
    }
    if (false || runAll) // deque
    {
        print_header("Deque");
        // 分段连续内存，动态调整大小
        // the elements of a deque can be scattered in different chunks of storage
        // efficient insertion/removal at the beginning and end of the sequence and random access
        // 功能和 vector 比较接近，但 deque 额外支持在头部动态增删元素
        std::deque<int> d = {7, 5, 16, 8};
        d.push_front(1);
        d.push_back(1);
        d.pop_front();
        d.pop_back();
    }
    if (false || runAll) // list
    {
        print_header("List");
        // 双向链表,不支持随机反问
        std::list<int> l = {7, 5, 16, 8};
        // Add an integer to the front of the list
        l.push_front(25);
        // Add an integer to the back of the list
        l.push_back(13);
        std::list<int>::iterator it = std::find(l.begin(), l.end(), 16);
        if (it != l.end())
        {
            l.insert(it, 10);
        }
        std::cout << now();
        for (auto i : l)
        {
            std::cout << i << ", ";
        }
        std::cout << std::endl;
    }
    if (false || runAll) // forward_list
    {
        print_header("forward_list");
        // 单链表，forward_list 是一个最小链表设计，它甚至没有size()接口，因为内部维护一个size变量会降低增删元素的效率。如果想要获取 forward_list 的 size，一个通常的做法是，用 std::distance 计算 begin 到 end 的距离得出 size。一句话总结：list 兼顾了接口丰富性牺牲了效率，而 forward_list 舍弃了不必要的接口只为追求极致效率。
    }

    // container adaptors
    auto printTop = [](auto &q) -> void
    {
        std::cout << now() << "Output: ";
        for (; !q.empty(); q.pop())
        {
            std::cout << q.top() << ", ";
        }
        std::cout << std::endl;
    };
    if (false || runAll) // queue
    {
        print_header("Queue, FIFO");
        std::deque<int> deq{1, 2, 3};
        std::queue<int> q(deq);
        q.push(4);
        INFO_STREAM("Input: 1, 2, 3, 4");
        INFO_STREAM("Top of queue: " << q.front());
        std::cout << now() << "Output: ";
        for (; !q.empty(); q.pop())
        {
            std::cout << q.front() << ", ";
        }
        std::cout << std::endl;
    }
    if (false || runAll) // priority_queue
    {
        print_header("priority_queue");
        // 在内部维护一个基于二叉树的大顶堆数据结构，在这个数据结构中，最大的元素始终位于堆顶部，且只有堆顶部的元素（max heap element）才能被访问和获取
        std::priority_queue<int> q;
        q.push(3);
        q.push(2);
        q.push(4);
        INFO_STREAM("Input: 3, 2, 4");
        printTop(q); // 4, 3, 2

        std::priority_queue<int, std::vector<int>, std::greater<int>> rq;
        rq.push(3);
        rq.push(2);
        rq.push(4);
        INFO_STREAM("Input 3, 2, 4");
        printTop(rq); // 2, 3, 4

        auto cmp = [](int left, int right)
        { return (left ^ 1) < (right ^ 1); }; // 异或
        std::priority_queue<int, std::vector<int>, decltype(cmp)> lambda_priority_queue(cmp);
        for (int i = 0; i < 10; ++i)
        {
            lambda_priority_queue.push(i);
        }
        INFO_STREAM("Input 1, 2, 3, ..., 9");
        printTop(lambda_priority_queue); // Output: 8, 9, 6, 7, 4, 5, 2, 3, 0, 1
    }
    if (false || runAll) // stack
    {
        print_header("Stack, LIFO");
        // LIFO
        std::stack<int> s;
        s.push(3);
        s.push(2);
        s.push(4);
        INFO_STREAM("Input: 3, 2, 4");
        printTop(s); // 4, 3, 2
    }

    // associative containers, 非线性的树结构，更准确的说是二叉树结构
    auto printMap = [](auto &m) -> void
    {
        std::cout << now();
        for (const auto &[key, value] : m)
        {
            std::cout << '[' << key << "] = " << value << "; ";
        }
        std::cout << std::endl;
    };
    if (false || runAll) // map
    {
        print_header("Map");
        // map 通过底层的「红黑树」数据结构来将所有的元素按照 key 的相对大小进行排序
        std::map<int, int> m;
        m.emplace(1, 2);
        printMap(m);
        INFO_STREAM("No error with m[2]: " << m[2]);
    }
    if (false || runAll) // multimap
    {
        print_header("Multimap");
        // multimap 与 map 底层原理完全一样，都是使用「红黑树」对元素数据按 key 的比较关系，进行快速的插入、删除和检索操作；所不同的是 multimap 允许将具有相同 key 的不同元素插入容器
        std::multimap<int, int> m1 = {{4, 4}, {3, 2}, {4, 4}, {3, 3}, {3, 1}};
        std::cout << now();
        auto n = m1.size();
        // 遍历
        for (auto const &p : m1)
        {
            std::cout << '[' << p.first << ":" << p.second << (--n ? "], " : "]");
        }
        std::cout << " }" << std::endl;

        // 搜索
        auto it = m1.find(3);
        std::cout << now() << "Search for key 3: ";
        // 同一个key的键值对会在一起
        while (it != m1.end() && it->first == 3)
        {
            std::cout << it->second;
            ++it;
        }
        std::cout << std::endl;

        auto range = m1.equal_range(4);
        std::cout << now() << "Search for key 3: ";
        for (auto itr = range.first; itr != range.second; ++itr)
        {
            std::cout << itr->second << " ";
        }
        std::cout << std::endl;
    }
    if (false || runAll) // set
    {
        print_header("Set");
        // 底层数据结构为红黑树，有序，不重复, 元素自身即key
        std::set<int> myset;
        for (int i = 1; i < 10; i++)
            myset.insert(i * 10);
    }
    if (false || runAll) // multiset
    {
    }
    if (false || runAll) // unordered_map
    {
        print_header("unordered_map");
        std::unordered_map<std::string, std::string> u =
        {
            {"RED", "#FF0000"},
            {"GREEN", "#00FF00"},
            {"BLUE", "#0000FF"}
        };
        printMap(u);
    }
}