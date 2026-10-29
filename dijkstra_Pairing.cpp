#include "dijkstra_Pairing.hpp"
#include <utility>

PairingHeap::PairingHeap(int n) : nodes(n), root(nullptr) {}

bool PairingHeap::empty() const {
    return root == nullptr;
}

bool PairingHeap::contains(int v) const {
    return nodes[v].inHeap;
}

// join two heap roots: the larger root becomes the first child of the smaller
PairingHeap::Node* PairingHeap::meld(Node* a, Node* b) {
    if (a == nullptr) {
        return b;
    }
    if (b == nullptr) {
        return a;
    }
    if (b->key < a->key) {
        std::swap(a, b);
    }
    // b becomes a's first child
    b->prev = a;
    b->sibling = a->child;
    if (a->child != nullptr) {
        a->child->prev = b;
    }
    a->child = b;
    a->prev = nullptr;
    a->sibling = nullptr;
    return a;
}

void PairingHeap::insert(int v, long long k) {
    Node* x = &nodes[v];
    *x = Node();
    x->key = k;
    x->vertex = v;
    x->inHeap = true;
    root = meld(root, x);
}

// cut x's whole subtree away from its parent and meld it back at the root
void PairingHeap::decreaseKey(int v, long long newKey) {
    Node* x = &nodes[v];
    x->key = newKey;
    if (x == root) {
        return;
    }

    // unlink x (prev is the parent if x is the first child, else a sibling)
    if (x->prev->child == x) {
        x->prev->child = x->sibling;
    } else {
        x->prev->sibling = x->sibling;
    }
    if (x->sibling != nullptr) {
        x->sibling->prev = x->prev;
    }
    x->prev = nullptr;
    x->sibling = nullptr;

    root = meld(root, x);
}

int PairingHeap::extractMin() {
    Node* z = root;

    // detach the children of z, each becomes its own tree
    buffer.clear();
    Node* c = z->child;
    while (c != nullptr) {
        Node* next = c->sibling;
        c->prev = nullptr;
        c->sibling = nullptr;
        buffer.push_back(c);
        c = next;
    }
    z->child = nullptr;

    // pass 1: meld the trees in pairs, left to right
    size_t paired = 0;
    for (size_t i = 0; i < buffer.size(); i += 2) {
        if (i + 1 < buffer.size()) {
            buffer[paired++] = meld(buffer[i], buffer[i + 1]);
        } else {
            buffer[paired++] = buffer[i];
        }
    }

    // pass 2: meld the results into one tree, right to left
    Node* result = nullptr;
    for (size_t i = paired; i > 0; --i) {
        result = meld(buffer[i - 1], result);
    }
    root = result;

    z->inHeap = false;
    return z->vertex;
}


dijkstra_Results dijkstra_B(const Graph& graph, int source) {
    const int n = graph.size();
    dijkstra_Results r{std::vector<long long>(n, INF), std::vector<int>(n, -1)};
    PairingHeap heap(n);

    r.dist[source] = 0;
    heap.insert(source, 0);
    COUNT(inserts);

    while (!heap.empty()) {
        int u = heap.extractMin();
        COUNT(extracts);

        for (const Edge& e : graph[u]) {
            long long nd = r.dist[u] + e.weight;
            if (nd < r.dist[e.vertex]) {
                r.dist[e.vertex] = nd;
                r.parent[e.vertex] = u;

                if (heap.contains(e.vertex)) {
                    heap.decreaseKey(e.vertex, nd);
                    COUNT(decreaseKeys);
                } else {
                    heap.insert(e.vertex, nd);
                    COUNT(inserts);
                }
            }
        }
    }
    return r;
}
