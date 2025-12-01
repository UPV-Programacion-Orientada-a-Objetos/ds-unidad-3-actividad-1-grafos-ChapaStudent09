// sparse_graph.cpp
#include "sparse_graph.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <sys/resource.h> // opcional para medición en linux

GrafoDisperso::GrafoDisperso() : n(0), m(0), construido(false) {}

GrafoDisperso::~GrafoDisperso() {}

size_t GrafoDisperso::estimate_memory_bytes(idx_t n, edge_count_t m) {
    size_t bytes = 0;
    bytes += (size_t)(n + 1) * sizeof(edge_count_t); // row_ptr
    bytes += (size_t)m * sizeof(idx_t); // col_idx
    bytes += (size_t)n * sizeof(edge_count_t) * 2; // in/out degrees
    return bytes;
}

void GrafoDisperso::construirCSR(idx_t max_node, const std::vector<std::pair<idx_t, idx_t>> &edges, bool dirigido) {
    n = max_node + 1;
    // si nodos aislados: consideramos todos hasta max_node
    m = edges.size();
    // contar grados out
    out_degree.assign(n, 0);
    in_degree.assign(n, 0);

    for (const auto &e : edges) {
        idx_t u = e.first, v = e.second;
        out_degree[u]++;
        in_degree[v]++;
        if (!dirigido) { // si no dirigido, añadir la inversa
            out_degree[v]++;
            in_degree[u]++;
        }
    }

    // construir row_ptr
    row_ptr.assign(n + 1, 0);
    for (idx_t i = 0; i < n; ++i)
        row_ptr[i+1] = row_ptr[i] + out_degree[i];

    // tamaño total de col_idx depende si dirigido o no
    edge_count_t total_edges = row_ptr[n];
    col_idx.assign((size_t)total_edges, 0);

    // posición temporal para insertar
    std::vector<edge_count_t> insert_pos(n);
    for (idx_t i = 0; i < n; ++i) insert_pos[i] = row_ptr[i];

    for (const auto &e : edges) {
        idx_t u = e.first, v = e.second;
        col_idx[insert_pos[u]++] = v;
        if (!dirigido) {
            col_idx[insert_pos[v]++] = u;
        }
    }

    construido = true;
}

LoadStats GrafoDisperso::cargarDesdeEdgeList(const std::string &path, bool dirigido) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "[C++ Core] ERROR abriendo archivo: " << path << std::endl;
        return {0,0,0,0};
    }
    std::cout << "[C++ Core] Inicializando GrafoDisperso..." << std::endl;
    auto t0 = std::chrono::high_resolution_clock::now();

    std::string line;
    std::vector<std::pair<idx_t, idx_t>> edges;
    edges.reserve(1000000); // reserva inicial
    idx_t max_node = 0;
    edge_count_t edge_counter = 0;

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') continue; // comentarios comunes en datasets SNAP
        std::istringstream iss(line);
        idx_t u, v;
        if (!(iss >> u >> v)) continue;
        edges.emplace_back(u, v);
        edge_counter++;
        if (u > max_node) max_node = u;
        if (v > max_node) max_node = v;
    }
    in.close();

    // construir CSR
    construirCSR(max_node, edges, dirigido);

    auto t1 = std::chrono::high_resolution_clock::now();
    double seconds = std::chrono::duration<double>(t1 - t0).count();
    size_t mem_est = estimate_memory_bytes(n, row_ptr[n]);

    std::cout << "[C++ Core] Carga completa. Nodos: " << n << " | Aristas (salientes totales): " << row_ptr[n] << std::endl;
    std::cout << "[C++ Core] Estructura CSR construida. Memoria estimada: " << (mem_est/1024.0/1024.0) << " MB." << std::endl;
    return {n, row_ptr[n], seconds, mem_est};
}

idx_t GrafoDisperso::nodoConMayorGrado(bool considerar_entrada, bool considerar_salida) {
    idx_t best_node = 0;
    edge_count_t best_deg = 0;
    for (idx_t i = 0; i < n; ++i) {
        edge_count_t deg = 0;
        if (considerar_salida) deg += out_degree[i];
        if (considerar_entrada) deg += in_degree[i];
        if (deg > best_deg) {
            best_deg = deg;
            best_node = i;
        }
    }
    std::cout << "[C++ Core] Nodo con mayor grado: " << best_node << " | grado: " << best_deg << std::endl;
    return best_node;
}

std::vector<idx_t> GrafoDisperso::getVecinos(idx_t nodo) {
    std::vector<idx_t> res;
    if (!construido) return res;
    if (nodo >= n) return res;
    edge_count_t start = row_ptr[nodo];
    edge_count_t end = row_ptr[nodo+1];
    res.reserve(end - start);
    for (edge_count_t i = start; i < end; ++i) {
        res.push_back(col_idx[(size_t)i]);
    }
    return res;
}

std::vector<idx_t> GrafoDisperso::bfs(idx_t inicio, idx_t max_depth) {
    std::vector<idx_t> visited_nodes;
    if (!construido) return visited_nodes;
    if (inicio >= n) return visited_nodes;

    std::vector<int> vis(n, -1); // -1 = no visitado, >=0 = depth
    std::queue<idx_t> q;
    q.push(inicio);
    vis[inicio] = 0;
    visited_nodes.push_back(inicio);

    while (!q.empty()) {
        idx_t u = q.front(); q.pop();
        int depth = vis[u];
        if ((idx_t)depth >= max_depth) continue;
        edge_count_t start = row_ptr[u];
        edge_count_t end = row_ptr[u+1];
        for (edge_count_t i = start; i < end; ++i) {
            idx_t v = col_idx[(size_t)i];
            if (vis[v] == -1) {
                vis[v] = depth + 1;
                visited_nodes.push_back(v);
                q.push(v);
            }
        }
    }
    std::cout << "[C++ Core] BFS completado. Inicio: " << inicio << " | nodos encontrados: " << visited_nodes.size() << std::endl;
    return visited_nodes;
}
