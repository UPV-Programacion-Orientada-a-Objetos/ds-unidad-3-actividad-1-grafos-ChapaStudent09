// sparse_graph.h
#ifndef SPARSE_GRAPH_H
#define SPARSE_GRAPH_H

#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <chrono>
#include <iostream>
#include <limits>

using idx_t = uint32_t;  // usa 32-bit para ahorrar memoria (hasta ~4e9 nodos)
using edge_count_t = uint64_t;

struct LoadStats {
    idx_t num_nodes;
    edge_count_t num_edges;
    double seconds;
    size_t mem_bytes_estimate;
};

class GrafoBase {
public:
    virtual ~GrafoBase() = default;
    virtual LoadStats cargarDesdeEdgeList(const std::string &path, bool dirigido=true) = 0;
    virtual idx_t nodoConMayorGrado(bool considerar_entrada, bool considerar_salida) = 0;
    virtual std::vector<idx_t> bfs(idx_t inicio, idx_t max_depth) = 0;
    virtual std::vector<idx_t> getVecinos(idx_t nodo) = 0;
    virtual idx_t numNodos() const = 0;
    virtual edge_count_t numAristas() const = 0;
};

// Implementación CSR simple (sólo aristas con pesos implícitos =1)
class GrafoDisperso : public GrafoBase {
private:
    idx_t n; // número de nodos (0..n-1)
    edge_count_t m; // número de aristas
    // CSR para aristas salientes:
    std::vector<edge_count_t> row_ptr;   // tamaño n+1, indices en col_idx
    std::vector<idx_t> col_idx;          // tamaño m
    // Para grado de entrada, podemos mantener indegree vector:
    std::vector<edge_count_t> out_degree;
    std::vector<edge_count_t> in_degree;
    bool construido;

public:
    GrafoDisperso();
    virtual ~GrafoDisperso();

    virtual LoadStats cargarDesdeEdgeList(const std::string &path, bool dirigido=true) override;
    virtual idx_t nodoConMayorGrado(bool considerar_entrada=true, bool considerar_salida=true) override;
    virtual std::vector<idx_t> bfs(idx_t inicio, idx_t max_depth) override;
    virtual std::vector<idx_t> getVecinos(idx_t nodo) override;
    virtual idx_t numNodos() const override { return n; }
    virtual edge_count_t numAristas() const override { return m; }

private:
    // pasos privados
    void construirCSR(idx_t max_node, const std::vector<std::pair<idx_t, idx_t>> &edges, bool dirigido);
    static size_t estimate_memory_bytes(idx_t n, edge_count_t m);
};

#endif // SPARSE_GRAPH_H
