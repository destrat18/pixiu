import os 
import sys

# import networkx as nx

def kgr_to_tdp_problem(input_kgr_file, write_file):

    with open(input_kgr_file, 'r') as f:
        lines = f.readlines()
    
    # first line: n, m
    # second, third lines: skip 
    # m lines of edges

    n, m = lines[0].split(' ')

    n = int(n)
    m = int(m)

    edges = []
    for i in range(3, 3 + m):
        source, target, _ = lines[i].split(' ')
        edges.append((int(source), int(target)))

    edges = list(set(edges))

    # undirected graph
    # G = nx.Graph()

    # add vertices 1 to n inclusive
    # G.add_nodes_from(list(range(1, n + 1)))
    # G.add_edges_from(edges)

    # get connected components
    # components = list(nx.connected_components(G))

    # for each component, get a vertex from it 

    # representative_vertices = []
    # for component in components:
    #     representative_vertices.append(list(component)[0])

    # if len(representative_vertices) > 1:
    #     # add i -- i + 1 edges to the edges list to make graph connected
    #     for i in range(len(representative_vertices) - 1):
    #         edges.append((representative_vertices[0], representative_vertices[i + 1]))

    to_write = ""

    to_write += f"p tdp {n} {len(edges)}\n"

    for edge in edges:
        to_write += f"{edge[0]} {edge[1]}\n"

    with open(write_file, 'w') as f:
        f.write(to_write)

