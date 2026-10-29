// Generates a SYNTHETIC road-like graph in DIMACS .gr format.
//
//   ./gen_roadlike <vertices> <arcs> <seed> <out.gr>
//   ./gen_roadlike 23947347 58333344 42 USA-road-d.SYNTH.gr
//
// Construction: lay the vertices on a grid, take a random spanning tree
// (so the graph is connected), then add random extra grid edges until the
// requested arc count is reached. Every edge becomes two arcs with the same
// weight, like the DIMACS road files. Weights are log-normal "distances".
// This is NOT real road data: it only matches the size and average degree.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>

static std::vector<uint32_t> parentUF;

static uint32_t findRoot(uint32_t x) {
    while (parentUF[x] != x) {
        parentUF[x] = parentUF[parentUF[x]];   // path halving
        x = parentUF[x];
    }
    return x;
}

int main(int argc, char** argv) {
    if (argc < 5) {
        std::fprintf(stderr, "usage: %s <vertices> <arcs> <seed> <out.gr>\n", argv[0]);
        return 1;
    }
    const uint64_t V = std::strtoull(argv[1], nullptr, 10);
    const uint64_t arcs = std::strtoull(argv[2], nullptr, 10);
    const uint64_t seed = std::strtoull(argv[3], nullptr, 10);
    if (arcs % 2 != 0 || V < 2) {
        std::fprintf(stderr, "arcs must be even (each road is two arcs) and V >= 2\n");
        return 1;
    }
    const uint64_t edges = arcs / 2;            // undirected roads wanted
    if (edges < V - 1) {
        std::fprintf(stderr, "need at least V-1 roads to stay connected\n");
        return 1;
    }

    // grid layout: vertex id = row * W + col, only ids < V exist
    const uint64_t W = static_cast<uint64_t>(std::ceil(std::sqrt(static_cast<double>(V))));

    // all grid edges between existing vertices
    std::vector<std::pair<uint32_t, uint32_t>> cand;
    cand.reserve(2 * V);
    for (uint64_t v = 0; v < V; ++v) {
        uint64_t col = v % W;
        if (col + 1 < W && v + 1 < V) cand.push_back({(uint32_t)v, (uint32_t)(v + 1)});
        if (v + W < V) cand.push_back({(uint32_t)v, (uint32_t)(v + W)});
    }
    if (cand.size() < edges) {
        std::fprintf(stderr, "grid only has %zu edges, cannot reach %llu\n",
                     cand.size(), (unsigned long long)edges);
        return 1;
    }

    std::mt19937_64 rng(seed);
    std::shuffle(cand.begin(), cand.end(), rng);

    parentUF.resize(V);
    std::iota(parentUF.begin(), parentUF.end(), 0u);

    std::lognormal_distribution<double> lengthDist(5.5, 1.0);   // median ~245, mean ~400

    FILE* out = std::fopen(argv[4], "w");
    if (!out) { std::perror("open output"); return 1; }
    static char buf[1 << 20];
    std::setvbuf(out, buf, _IOFBF, sizeof buf);

    std::fprintf(out, "c Synthetic road-like graph (not real road data)\n");
    std::fprintf(out, "p sp %llu %llu\n", (unsigned long long)V, (unsigned long long)arcs);

    uint64_t treeEdges = 0, extraEdges = 0, written = 0;
    for (const auto& e : cand) {
        uint32_t a = findRoot(e.first), b = findRoot(e.second);
        bool take = false;
        if (a != b) {                       // joins two components: spanning tree edge
            parentUF[a] = b;
            ++treeEdges;
            take = true;
        } else if (written < edges && (edges - written) > (V - 1 - treeEdges)) {
            ++extraEdges;                   // extra road, only if we still have room
            take = true;
        }
        if (!take) continue;

        long long w = 1 + static_cast<long long>(lengthDist(rng));
        if (w > 100000) w = 100000;
        std::fprintf(out, "a %u %u %lld\na %u %u %lld\n",
                     e.first + 1, e.second + 1, w, e.second + 1, e.first + 1, w);
        ++written;
        if (written == edges && treeEdges == V - 1) break;
    }
    std::fclose(out);

    if (treeEdges != V - 1 || written != edges) {
        std::fprintf(stderr, "ERROR: tree=%llu (want %llu), written=%llu (want %llu)\n",
                     (unsigned long long)treeEdges, (unsigned long long)(V - 1),
                     (unsigned long long)written, (unsigned long long)edges);
        return 1;
    }
    std::printf("wrote %llu vertices, %llu arcs (%.3f arcs per vertex), %llu tree roads + %llu extra roads\n",
                (unsigned long long)V, (unsigned long long)(2 * written),
                (double)(2 * written) / V, (unsigned long long)treeEdges,
                (unsigned long long)extraEdges);
    return 0;
}
