# graph_wrapper.pxd
# Definición de la clase C++ para Cython
cdef extern from "../cpp/include/sparse_graph.h" namespace "":
    ctypedef unsigned int idx_t
    ctypedef unsigned long long edge_count_t

    cdef cppclass GrafoBase:
        GrafoBase() except +
        virtual LoadStats cargarDesdeEdgeList(const std::string &path, bool dirigido=true) except +
        virtual idx_t nodoConMayorGrado(bool considerar_entrada=true, bool considerar_salida=true) except +
        virtual std::vector[idx_t] bfs(idx_t inicio, idx_t max_depth) except +
        virtual std::vector[idx_t] getVecinos(idx_t nodo) except +
        virtual idx_t numNodos() const except +
        virtual edge_count_t numAristas() const except +

    cdef cppclass GrafoDisperso(GrafoBase):
        GrafoDisperso() except +
        ~GrafoDisperso() except +
        LoadStats cargarDesdeEdgeList(const std::string &path, bool dirigido=true) except +
        idx_t nodoConMayorGrado(bool considerar_entrada=true, bool considerar_salida=true) except +
        std::vector[idx_t] bfs(idx_t inicio, idx_t max_depth) except +
        std::vector[idx_t] getVecinos(idx_t nodo) except +
        idx_t numNodos() const except +
        edge_count_t numAristas() const except +

    cdef struct LoadStats:
        idx_t num_nodes
        edge_count_t num_edges
        double seconds
        size_t mem_bytes_estimate
