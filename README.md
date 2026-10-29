# Dijkstra-Priority-Queues

A C++ comparison of two priority queues inside Dijkstra's shortest-path algorithm, built to test whether the pairing heap's better asymptotic bound for decrease-key gives faster running times than a binary heap.

- Algorithm A: binary heap (std::priority_queue) with lazy deletion. It has no decrease-key, so a shorter path pushes a duplicate entry and the stale one is skipped when it is popped.
- Algorithm B: hand-written pairing heap with a real decrease-key. Each vertex is in the heap at most once.

Both use the same adjacency-list graph, the same relaxation logic and the same result structure, so only the priority queue differs.

Build (timing build):
    g++ -std=c++23 -O2 -Wall -Wextra benchmark.cpp dijkstra_Binary.cpp dijkstra_Pairing.cpp -o bench

This produces an executable called 'bench' in the project directory. The flags are:

- std=c++23: required language standard.
- O2: optimisation level 2. Timing an unoptimised build would be misleading, because it slows the pointer-based pairing heap and the array-based binary heap by different amounts.
- Wall -Wextra: enables extra compiler warnings.

No sanitizers are used for timing, because they slow the two algorithms by different amounts. They are only used for the tests below.

Other builds:

    # operation counts (pushes, stale pops, inserts, decrease-keys, extract-mins)
    g++ -std=c++23 -O2 -Wall -Wextra -DCOUNT_OPS benchmark.cpp dijkstra_Binary.cpp dijkstra_Pairing.cpp -o test

    # synthetic graph generator
    g++ -std=c++17 -O2 gen_roadlike.cpp -o gen_roadlike

The counting build adds a little overhead to every run, so only use it for the counts, never for the timings.

Usage
    ./bench <file>.gr [more files .gr]
    ./test <file>.gr
    ./gen_roadlike <vertices> <arcs> <seed> <output file>

Examples:
    ./bench USA-road-d.NY.gr
    ./test USA-road-d.NY.gr 
    ./gen_roadlike 321270 800172 42 SYNTH-BAY.gr

bench reads each .gr file, runs both algorithms from 30 random source vertices and prints one line per graph:

    File Name = maps/USA-road-d.NY.gr
    V = 264346
    E = 733846
    E/V = 2.8
    A = 31.047ms
    B = 50.489ms
    B/A = 1.63 OK
    IQR A = [28.590, 32.934]ms
    IQR B = [48.680, 52.494]ms

The times are medians. B/A above 1 means the binary heap (A) was faster. OK means both algorithms gave identical distances on every run, MISMATCH! means they did not.

Dependencies
A C++23-capable compiler (tested with g++ 13.3.0 on Ubuntu 24.04 under WSL).

Files
- dijkstra_Binary.hpp/.cpp: Algorithm A, the shared graph types (Edge, Graph, INF) and reconstructPath
- dijkstra_Pairing.hpp/.cpp: Algorithm B, the pairing heap
- benchmark.cpp: loads .gr files, times both algorithms and checks the distances match
- gen_roadlike.cpp: generator for synthetic road-like graphs


Data
The graphs are in the DIMACS .gr format. Lines starting with 'c' are comments, the 'p sp <vertices> <arcs>' line is the header, and each 'a <from> <to> <weight>' line is one directed arc. Vertices are numbered from 1 in the file and shifted to start from 0 when loaded.

New York (real data): USA-road-d.NY.gr, the New York distance graph from the 9th DIMACS Implementation Challenge (264,346 vertices, 733,846 arcs). It is not included in this repository. Download it from https://www.diag.uniroma1.it/challenge9/download.shtml and unzip it:
    gunzip USA-road-d.NY.gr.gz

Synthetic graphs: the DIMACS site was unreachable when the other regions were needed, so five graphs were generated with the same vertex and arc counts as the Bay Area, Colorado, Florida, Northeast and Great Lakes road networks. They are NOT DIMACS data. They share the real networks' size and density but not their structure. gen_roadlike lays the vertices on a grid, builds a random spanning tree so the graph is connected, then adds random extra grid edges until the arc count is reached. Every road is two arcs with equal weight, and the weights are log-normal. The seed is fixed, so the files are reproducible:
    ./gen_roadlike 321270  800172  42 SYNTH-BAY.gr
    ./gen_roadlike 435666  1057066 42 SYNTH-COL.gr
    ./gen_roadlike 1070376 2712798 42 SYNTH-FLA.gr
    ./gen_roadlike 1524453 3897636 42 SYNTH-NE.gr
    ./gen_roadlike 2758119 6885658 42 SYNTH-LKS.gr

Method
- Time is measured with std::chrono::steady_clock around the Dijkstra call only, so loading the graph and printing are not timed. Each run explores the whole graph from one source, so there is no target.
- 30 source vertices per graph, chosen with std::mt19937_64 (seed 42). Each (graph, source) pair is timed once per algorithm, after one untimed warm-up run of each.
- The reported value is the median, with the interquartile range as the spread. The order of A and B alternates between sources.
- Both algorithms run on the same graph object and the same sources, and their distances are compared after every pair of runs.

Results (median over 30 sources, ms)

    Graph                    A         B         B/A
    New York (real)          27.112    47.637    1.76
    Bay Area (synthetic)     43.591    72.232    1.66
    Colorado (synthetic)     66.805    105.917   1.59
    Florida (synthetic)      214.133   343.542   1.60
    Northeast (synthetic)    312.690   499.621   1.60
    Great Lakes (synthetic)  598.264   997.565   1.67

A was faster on all six graphs.

Hardware: AMD Ryzen 5800X (8 cores), 32 GB DDR4-3200, Windows 10 with Ubuntu 24.04 (WSL), g++ 13.3.0.

Known limitations
Only one graph is real data (New York). The other five are synthetic and should be reported separately from it.

Every graph is sparse (about 2.5 arcs per vertex), so none of them tests the dense case where the pairing heap's cheaper decrease-key should matter most, and no crossover point was found.

Algorithm A uses the standard library's heavily tuned std::priority_queue, whereas Algorithm B is a hand-written implementation, so implementation effort is not equal. This may favour A.

Operation counts come from the first source only, and cache misses and memory use were not measured, so the explanation for B's higher cost per operation is consistent with the results but not proven.

Results come from one machine under WSL.

AI use
AI was used to make this project. Claude (Anthropic) helped write and explain the code (the heap implementations, the benchmark, the tests and the graph generator), helped draft and review the report, and generated the synthetic graphs. I chose the topic and the two algorithms, ran the benchmarks, and checked and edited the results and the code. The report has a fuller AI use section, including examples where the AI was wrong.
