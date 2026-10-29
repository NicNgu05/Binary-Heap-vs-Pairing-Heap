#pragma once
#include <vector>
#include <limits>

struct Edge {
    int vertex;
    long long weight;
};

using Graph = std::vector<std::vector<Edge>>;
constexpr long long INF = std::numeric_limits<long long>::max();

struct dijkstra_Results {
    std::vector<long long> dist;
    std::vector<int> parent;   // -1 = no parent
};

class MinHeap {
public:
    MinHeap(int n);   // node ids are 0..n-1
    bool empty();
    bool contains(int v);
    void insert(int v, long long key);
    void decreaseKey(int v, long long newKey);
    int  extractMin();

private:
    std::vector<int> heap;        // heap[i] = node at heap index i
    std::vector<int> pos;         // pos[v] = index of v in heap, or -1
    std::vector<long long> key;   // key[v] = current priority of v

    void swapNodes(int i, int j);
    void siftUp(int i);
    void siftDown(int i);
};

struct OpCounts {
    long long pushes = 0, stalePops = 0;
    long long inserts = 0, decreaseKeys = 0, extracts = 0;
};
extern OpCounts opCounts;

#ifdef COUNT_OPS
#define COUNT(field) (++opCounts.field)
#else
#define COUNT(field) ((void)0)
#endif

dijkstra_Results dijkstra_A(const Graph& g, int source);