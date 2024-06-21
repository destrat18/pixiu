# example input:
# 4 1
# 1 4 2 3
# 1 1 1 1
# 3 4 c
# a
# b
# c
# d
# 2

# first line: n, m
# second line costs: c1, c2, ..., cn
# third line votes: v1, v2, ..., vn
# fourth line: m edges of the form: i j c
# -- c is d for dependency, c is c for conflict
# n line for names of candidates
# last line: either V max volume of the knapsack or nothing, then V = 90112 

from scipy.optimize import LinearConstraint
from scipy.optimize import milp
import numpy as np
import pandas as pd
from copy import copy
import os

MAX_VOLUME = 90112

class Instance:
    def __init__(self):
        self.number_of_vertices = 0
        self.number_of_edges = 0
        self.costs = []
        self.volumes = []
        self.edges = [] # list of tuples
        self.index_to_hash = dict()
        self.hash_to_index = dict()
        self.max_volume = 0
        self.block_height = 0
    
    def __str__(self):
        return 'Instance(number_of_vertices={}, number_of_edges={}, costs={}, volumes={}, edges={}, index_to_hash={}, hash_to_index={}, max_volume={})'.format(self.number_of_vertices, self.number_of_edges, self.costs, self.volumes, self.edges, self.index_to_hash, self.hash_to_index, self.max_volume)


def parse_from_nodes_edges(nodes_df, edges_df):
    # nodes_df: hash,fees,size
    # edges_df: source,target,type


    hash_fees_size = nodes_df.values
    hash_fees_size = hash_fees_size[hash_fees_size[:, 0].argsort()] # sorted by hash
    print(hash_fees_size)

    hash_to_index = dict()
    index_to_hash = dict()
    
    for i, (hash, fees, size) in enumerate(hash_fees_size):
        hash_to_index[hash] = i
        index_to_hash[i] = hash

    costs = hash_fees_size[:, 1].astype(int)
    volumes = hash_fees_size[:, 2].astype(int)

    number_of_vertices = len(hash_fees_size)
    number_of_edges = len(edges_df)
    max_volume = MAX_VOLUME

    edges = []
    for i, (source, target, type) in enumerate(edges_df.values):
        edges.append((hash_to_index[source], hash_to_index[target], type))

    instance = Instance()
    instance.number_of_vertices = copy(number_of_vertices)
    instance.number_of_edges = copy(number_of_edges)
    instance.costs = copy(costs)
    instance.volumes = copy(volumes)
    instance.edges = edges
    instance.index_to_hash = index_to_hash
    instance.hash_to_index = hash_to_index
    instance.max_volume = max_volume

    return instance
    
def instance_from_path_number(path, height):
    node_file_name = 'tx_pool_{}_nodes.csv'.format(height)
    edge_file_name = 'tx_pool_{}_edges.csv'.format(height)

    node_file_path = os.path.join(path, node_file_name)
    edge_file_path = os.path.join(path, edge_file_name)

    df_nodes = pd.read_csv(node_file_path)
    df_edges = pd.read_csv(edge_file_path)

    instance = parse_from_nodes_edges(df_nodes, df_edges)
    instance.block_height = height

    return instance


def parse_from_str(inp):
    lines = inp.split('\n')
    # clean up empty lines
    lines = [line.strip() for line in lines if line.strip() != '']
    n, m = map(int, lines[0].split())
    costs = list(map(int, lines[1].split()))
    volumes = list(map(int, lines[2].split()))
    edges = []
    for i in range(m):
        fr, to, ty = lines[3 + i].split()
        edges.append((int(fr) - 1, int(to) - 1, ty))
        # edges.append(lines[3 + i].split())
    names = lines[3 + m: 3 + m + n]
    V = int(lines[3 + m + n]) if 3 + m + n < len(lines) else 90112
    
    parsed = dict()
    parsed['n'] = n
    parsed['m'] = m
    parsed['costs'] = costs
    parsed['volumes'] = volumes
    parsed['edges'] = edges
    parsed['names'] = names
    parsed['V'] = V

    return parsed

def parse_from_file(filename):
    with open(filename, 'r') as f:
        inp = f.read()
    ans = parse_from_str(inp)
    ans['filename'] = filename
    return ans


def solve(filename):
    parsed = parse_from_file(filename)
    c = -np.array(parsed['costs'])
    print("Costs: ", -np.sum(c))
    max_volume = parsed['V']

    A = []
    b_u = []
    b_l = []

    # constrain on the volume
    b_l.append(0)
    b_u.append(max_volume)
    A.append(np.array(parsed['volumes']))

    # constrain on the edges
    for fr, to, ty in parsed['edges']:
        if ty == 'c': # 0 <= fr + to <= 1
            b_l.append(0)
            b_u.append(1)
            # append row with 1 in fr and to
            row = np.zeros(parsed['n'])
            row[fr] = 1
            row[to] = 1
            A.append(row)
        elif ty == 'd': # if we include to, we must include fr: 2 >= fr - to >= 0
            b_l.append(0)
            b_u.append(2)
            # append row with 1 in fr and -1 in to
            row = np.zeros(parsed['n'])
            row[fr] = -1
            row[to] = 1
            A.append(row)
        else:
            raise ValueError('Unknown edge type: {}'.format(ty))
        
    # add constraint that all variables are binary
    for i in range(parsed['n']):
        row = np.zeros(parsed['n'])
        row[i] = 1
        A.append(row)
        b_l.append(0)
        b_u.append(1)

    A = np.array(A, dtype=int)
    b_l = np.array(b_l, dtype=int)
    b_u = np.array(b_u, dtype=int)

    integrality = np.ones_like(c, dtype=int)
    constraints = LinearConstraint(A, b_l, b_u)
    print("Solver started")
    res = milp(c=c, constraints=constraints, integrality=integrality)
    print("Solver finished")

    opt_val = -res.fun
    opt_vec = res.x
    ans = dict()

    ans['fun'] = np.rint(opt_val)
    ans['x'] = opt_vec

    return ans

def rounding(val):
    return int(val + 0.5)

def print_solution(solution, parsed):
    # print filename optvalue n_ones 
    # the next n lines print hashes
    ans = ""
    ans += parsed['filename'] + " " + str(rounding(solution["fun"])) + " " + str(rounding(np.sum(solution["x"]))) + "\n"
    for i in range(parsed['n']):
        if solution["x"][i] > 0.5:
            ans += "# " + parsed['names'][i] + "\n"
    return ans

def get_solution_tuple(solution, parsed):
    filepath = parsed['filename']
    # get filename from filepath
    filename = filepath.split('/')[-1]
    # split by underscore
    filename = filename.split('_')[-1]
    # remove extension
    filename = filename.split('.')[0]

    hashes_semicolon_separated = ';'.join([parsed['names'][i] for i in range(parsed['n']) if solution['x'][i] > 0.99])

    opt_value = rounding(solution['fun'])
    n_ones = rounding(np.sum(solution['x']))

    ans = dict()
    ans['filename'] = filename
    ans['opt_value'] = opt_value
    ans['n_ones'] = n_ones
    ans['hashes_semicolon_separated'] = hashes_semicolon_separated

    return ans

if __name__ == '__main__':
    import sys
    import os
    if len(sys.argv) < 2:
        print('Usage: python3 solveilp.py <filename>')
        exit(1)
    filename = sys.argv[1]

    ans = solve(filename)
    print(ans['fun'])
    