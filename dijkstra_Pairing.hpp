#pragma once
#include "dijkstra_Binary.hpp"   // Edge, Graph, INF, dijkstra_Results
#include <vector>

// Algorithm B: pairing heap with decrease-key
class PairingHeap {
public:
    PairingHeap(int n);   // node ids are 0..n-1
    bool empty() const;
    bool contains(int v) const;
    void insert(int v, long long k);
    void decreaseKey(int v, long long newKey);
    int  extractMin();

private:
    struct Node {
        long long key = INF;
        int vertex = -1;
        bool inHeap = false;
        Node* child = nullptr;     // first child
        Node* sibling = nullptr;   // next sibling
        Node* prev = nullptr;      // previous sibling, or parent if first child
    };

    std::vector<Node> nodes;       // nodes[v] is the node for vertex v
    std::vector<Node*> buffer;     // reused by extractMin
    Node* root;

    Node* meld(Node* a, Node* b);
};

dijkstra_Results dijkstra_B(const Graph& graph, int source);
