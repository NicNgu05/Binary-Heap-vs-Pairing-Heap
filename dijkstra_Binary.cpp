#include "dijkstra_Binary.hpp"
#include <algorithm>
#include <queue>
#include <utility>


dijkstra_Results dijkstra_A(const Graph& graph, int source) {
    const int n = graph.size();
    dijkstra_Results r{std::vector<long long>(n, INF), std::vector<int>(n, -1)};

    using State = std::pair<long long, int>;  // (dist, node)
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

    r.dist[source] = 0;
    pq.push({0, source});
    COUNT(pushes);

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        // stale entry: a shorter path to u was already found
        if (d > r.dist[u]) {
            COUNT(stalePops);
            continue;
        }

        for (const Edge& e : graph[u]) {
            long long nd = d + e.weight;
            if (nd < r.dist[e.vertex]) {
                r.dist[e.vertex] = nd;
                r.parent[e.vertex] = u;
                pq.push({nd, e.vertex});
                COUNT(pushes);
            }
        }
    }
    return r;
}
