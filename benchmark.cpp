// Benchmark: Algorithm A (binary heap, lazy) vs Algorithm B (pairing heap)
//
//   ./bench USA-road-d.NY.gr [more.gr ...]   benchmark DIMACS .gr files
//
// Build WITHOUT -DCOUNT_OPS for timing, WITH it to get operation counts.

#include "dijkstra_Binary.hpp"
#include "dijkstra_Pairing.hpp"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using DijkstraFn = dijkstra_Results (*)(const Graph&, int);
using Clock = std::chrono::steady_clock;
OpCounts opCounts;
// ---------------- graph loading ----------------

// DIMACS .gr file: lines "a <from> <to> <weight>", vertices numbered from 1
bool loadDimacs(const std::string& path, Graph& g) {
    FILE* f = std::fopen(path.c_str(), "r");
    if (!f) return false;

    char line[256];
    while (std::fgets(line, sizeof line, f)) {
        if (line[0] == 'p') {
            int n, m;
            if (std::sscanf(line, "p sp %d %d", &n, &m) == 2) g.assign(n, {});
        } else if (line[0] == 'a') {
            int u, v;
            long long w;
            if (std::sscanf(line, "a %d %d %lld", &u, &v, &w) == 3) {
                g[u - 1].push_back({v - 1, w});
            }
        }
    }
    std::fclose(f);
    return !g.empty();
}

long long countEdges(const Graph& g) {
    long long m = 0;
    for (const auto& adj : g) m += adj.size();
    return m;
}

// ---------------- timing ----------------

double timeMs(DijkstraFn f, const Graph& g, int src, dijkstra_Results& out) {
    auto t0 = Clock::now();
    out = f(g, src);
    auto t1 = Clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

double median(std::vector<double> v) {
    std::sort(v.begin(), v.end());
    return v[v.size() / 2];
}

double quartile(std::vector<double> v, double q) {
    std::sort(v.begin(), v.end());
    return v[static_cast<size_t>(q * (v.size() - 1))];
}

void benchmarkGraph(const std::string& label, const Graph& g, int numSources,
                    std::mt19937_64& rng) {
    const int n = static_cast<int>(g.size());
    const long long m = countEdges(g);

    std::vector<int> sources;
    for (int i = 0; i < numSources; ++i) sources.push_back(rng() % n);

    // warm-up (not timed)
    dijkstra_Results warm;
    timeMs(dijkstra_A, g, sources[0], warm);
    timeMs(dijkstra_B, g, sources[0], warm);

    std::vector<double> timesA, timesB;
    bool ok = true;

    for (size_t i = 0; i < sources.size(); ++i) {
        dijkstra_Results ra, rb;
        // alternate the order so neither algorithm always runs first
        if (i % 2 == 0) {
            timesA.push_back(timeMs(dijkstra_A, g, sources[i], ra));
            timesB.push_back(timeMs(dijkstra_B, g, sources[i], rb));
        } else {
            timesB.push_back(timeMs(dijkstra_B, g, sources[i], rb));
            timesA.push_back(timeMs(dijkstra_A, g, sources[i], ra));
        }
        if (ra.dist != rb.dist) ok = false;   // correctness check
    }

    double medA = median(timesA);
    double medB = median(timesB);

    std::printf("File Name = %s \nV = %d \nE = %lld \nE/V = %.1f \nA = %.3fms \nB = %.3fms \nB/A = %.2f %s",
            label.c_str(), n, m, static_cast<double>(m) / n,
            medA, medB, medB / medA, ok ? "OK" : "MISMATCH!");

    std::printf("\nIQR A = [%.3f, %.3f]ms\nIQR B = [%.3f, %.3f]ms",
            quartile(timesA, 0.25), quartile(timesA, 0.75),
            quartile(timesB, 0.25), quartile(timesB, 0.75));

#ifdef COUNT_OPS
    opCounts = OpCounts();
    dijkstra_A(g, sources[0]);
    long long pushes = opCounts.pushes;
    long long stale = opCounts.stalePops;
    opCounts = OpCounts();
    dijkstra_B(g, sources[0]);
    std::printf("  \nA: pushes = %lld stale = %lld  \nB: inserts = %lld decKeys = %lld extracts = %lld",
                pushes, stale, opCounts.inserts, opCounts.decreaseKeys, opCounts.extracts);
#endif
    std::printf("\n");
    std::fflush(stdout);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " graph.gr [more.gr ...]\n";
        return 1;
    }

    std::mt19937_64 rng(42);   // fixed seed -> same sources every run

#ifdef COUNT_OPS
    std::cout << "NOTE: built with -DCOUNT_OPS, timings include counter overhead\n";
#endif

    for (int i = 1; i < argc; ++i) {
        Graph g;
        if (!loadDimacs(argv[i], g)) {
            std::cerr << "could not load " << argv[i] << "\n";
            return 1;
        }
        benchmarkGraph(argv[i], g, 30, rng);
    }
    return 0;
}
