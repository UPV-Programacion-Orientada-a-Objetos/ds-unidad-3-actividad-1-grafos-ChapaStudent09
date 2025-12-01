# graph_wrapper.pyx
# distutils: language = c++

from libcpp.vector cimport vector
from libcpp.string cimport string
from cython.operator cimport dereference
cimport cython
cimport numpy as np
import numpy as np
from graph_wrapper cimport GrafoDisperso, LoadStats, idx_t

cdef class PyGrafo:
    cdef GrafoDisperso* thisptr
    def __cinit__(self):
        self.thisptr = new GrafoDisperso()
    def __dealloc__(self):
        if self.thisptr is not NULL:
            del self.thisptr
            self.thisptr = NULL

    def cargar(self, path: str, dirigido: bint = True):
        cdef string p = path.encode('utf-8')
        cdef LoadStats stats = self.thisptr.cargarDesdeEdgeList(p, dirigido)
        return {
            "num_nodes": int(stats.num_nodes),
            "num_edges": int(stats.num_edges),
            "seconds": float(stats.seconds),
            "mem_bytes_estimate": int(stats.mem_bytes_estimate)
        }

    def nodo_mayor_grado(self, considerar_entrada: bint = True, considerar_salida: bint = True):
        cdef idx_t n = self.thisptr.nodoConMayorGrado(considerar_entrada, considerar_salida)
        return int(n)

    def bfs(self, inicio: int, max_depth: int):
        cdef idx_t s = <idx_t>inicio
        cdef idx_t md = <idx_t>max_depth
        cdef vector[idx_t] res = self.thisptr.bfs(s, md)
        # convertir vector a lista python
        pylist = [int(res[i]) for i in range(res.size())]
        return pylist

    def vecinos(self, nodo: int):
        cdef idx_t v = <idx_t>nodo
        cdef vector[idx_t] res = self.thisptr.getVecinos(v)
        return [int(res[i]) for i in range(res.size())]

    def num_nodos(self):
        return int(self.thisptr.numNodos())

    def num_aristas(self):
        return int(self.thisptr.numAristas())
