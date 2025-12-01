# app.py (Streamlit GUI)
import streamlit as st
import time
import os
import networkx as nx
import matplotlib.pyplot as plt

# intenta importar la extensión compilada
try:
    import neuronet_core as core
except Exception as e:
    st.error("No se pudo importar la extensión C++ (neuronet_core). Compila primero con `python setup.py build_ext --inplace`")
    st.stop()

st.set_page_config(page_title="NeuroNet - Visualizador", layout="wide")

st.title("NeuroNet — Análisis y Visualización de Propagación")

with st.sidebar:
    st.header("Controles")
    uploaded = st.file_uploader("Cargar edge-list (formato: u v por línea)", type=["txt", "edges", "csv"])
    dirigido = st.checkbox("Grafo dirigido", value=True)
    btn_load = st.button("Cargar grafo (C++)")
    st.markdown("---")
    st.write("Búsqueda / simulación")
    start_node = st.number_input("Nodo inicio (id)", min_value=0, value=0)
    depth = st.number_input("Profundidad máxima (BFS)", min_value=1, value=2)
    btn_bfs = st.button("Ejecutar BFS")

# estado simple
if 'core_obj' not in st.session_state:
    st.session_state['core_obj'] = None
    st.session_state['loaded_info'] = None

col1, col2 = st.columns([1,2])

with col1:
    st.subheader("Operaciones")
    if btn_load:
        if uploaded is None:
            st.error("Sube primero un archivo edge-list.")
        else:
            # guardar temp file
            t0 = time.time()
            tmp_path = os.path.join(os.getcwd(), "uploaded_edges.txt")
            with open(tmp_path, "wb") as f:
                f.write(uploaded.getbuffer())
            st.info("Archivo guardado temporalmente: " + tmp_path)
            core_obj = core.PyGrafo()
            with st.spinner("Cargando en C++..."):
                info = core_obj.cargar(tmp_path, dirigido)
            st.session_state['core_obj'] = core_obj
            st.session_state['loaded_info'] = info
            st.success("Carga completa: {} nodos | {} aristas".format(info["num_nodes"], info["num_edges"]))
            st.write("Tiempo de carga (s):", info["seconds"])
            st.write("Memoria estimada (MB):", round(info["mem_bytes_estimate"]/1024/1024, 3))

    if st.session_state['loaded_info'] is not None:
        st.markdown("#### Métricas rápidas")
        info = st.session_state['loaded_info']
        st.write("Nodos:", info["num_nodes"])
        st.write("Aristas:", info["num_edges"])
        st.write("Tiempo carga (s):", info["seconds"])
        st.write("Mem est (MB):", round(info["mem_bytes_estimate"]/1024/1024, 3))
        nodo_critico = st.session_state['core_obj'].nodo_mayor_grado(True, True)
        st.write("Nodo con mayor grado (entrada+salida):", nodo_critico)

with col2:
    st.subheader("Visualización")
    if btn_bfs:
        if st.session_state['core_obj'] is None:
            st.error("Carga primero un grafo.")
        else:
            core_obj = st.session_state['core_obj']
            t0 = time.time()
            nodes = core_obj.bfs(int(start_node), int(depth))
            t1 = time.time()
            st.write(f"BFS completado: {len(nodes)} nodos en {round((t1-t0)*1000,3)} ms")
            # Construir subgrafo de los nodos encontrados (obtener aristas entre ellos)
            sub_nodes_set = set(nodes)
            G = nx.DiGraph() if dirigido else nx.Graph()
            G.add_nodes_from(nodes)
            # para cada nodo obtener sus vecinos y añadir arista si el vecino está en el set
            for u in nodes:
                vecinos = core_obj.vecinos(u)
                for v in vecinos:
                    if v in sub_nodes_set:
                        G.add_edge(u, v)
            # dibujar con matplotlib
            fig, ax = plt.subplots(figsize=(8,6))
            pos = nx.spring_layout(G, seed=42)
            nx.draw(G, pos=pos, with_labels=True, node_size=80, ax=ax)
            st.pyplot(fig)
