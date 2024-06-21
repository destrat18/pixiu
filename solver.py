from scipy.optimize import LinearConstraint
from scipy.optimize import milp
import numpy as np
import pandas as pd
from copy import copy
import os
import sys
import concurrent.futures


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
    # print(hash_fees_size)

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


def solve_instance(instance):
    c = -np.array(instance.costs)
    max_volume = instance.max_volume

    A = []
    b_u = []
    b_l = []

    # constrain on the volume
    b_l.append(0)
    b_u.append(max_volume)
    A.append(np.array(instance.volumes))

    # constrain on the edges
    for fr, to, ty in instance.edges:
        if ty == 'c': # 0 <= fr + to <= 1
            b_l.append(0)
            b_u.append(1)
            # append row with 1 in fr and to
            row = np.zeros(instance.number_of_vertices)
            row[fr] = 1
            row[to] = 1
            A.append(row)
        elif ty == 'd': # if we include to, we must include fr: 2 >= fr - to >= 0
            fr, to = to, fr
            b_l.append(0)
            b_u.append(2)
            # append row with 1 in fr and -1 in to
            row = np.zeros(instance.number_of_vertices)
            row[fr] = -1
            row[to] = 1
            A.append(row)
        else:
            raise ValueError('Unknown edge type: {}'.format(ty))
        
    # add constraint that all variables are binary
    for i in range(instance.number_of_vertices):
        row = np.zeros(instance.number_of_vertices)
        row[i] = 1
        A.append(row)
        b_l.append(0)
        b_u.append(1)

    A = np.array(A, dtype=int)
    b_l = np.array(b_l, dtype=int)
    b_u = np.array(b_u, dtype=int)

    integrality = np.ones_like(c, dtype=int)
    constraints = LinearConstraint(A, b_l, b_u)
    # print("Solver started")
    res = milp(c=c, constraints=constraints, integrality=integrality)
    # print("Solver finished")

    opt_val = -res.fun
    opt_vec = res.x
    ans = dict()

    ans['fun'] = np.rint(opt_val)
    ans['x'] = np.rint(opt_vec)
    ans['hashes'] = [instance.index_to_hash[i] for i in range(instance.number_of_vertices) if ans['x'][i] > 0.5]

    return ans


def get_block_height(file_name):
    return file_name.split('_')[2]


def get_heights_in_folder(folder_path):
    file_names = os.listdir(folder_path)
    file_names = [file_name for file_name in file_names if file_name.endswith('_nodes.csv')]
    heights = []
    for file_name in file_names:
        heights.append(get_block_height(file_name))
    return heights

def solve(directory, height):
    instance = instance_from_path_number(directory, height)
    solution = solve_instance(instance)
    return solution

if __name__ == '__main__':
    import sys
    import os
    if len(sys.argv) < 2:
        print('Usage: python3 solveilp.py <directory>')
        exit(1)

    
    directory = sys.argv[1]

    # check if folder exists
    is_directory = os.path.isdir(directory)
    if not is_directory:
        print('Given path is not a directory')
        exit(1)    

    # get heights in folder
    heights = get_heights_in_folder(directory)
    # heights = sorted(heights, key=lambda x: int(x))
    # print(heights)

    print(heights)
    solutions = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=10) as executor:
        
        future_to_height = {}
        # solve all problems
        for height in heights:
            future_to_height[executor.submit(solve, directory, height)] =  height
        

        for future in concurrent.futures.as_completed(future_to_height):
            height = future_to_height[future]
            try:
                solution = future.result()
                solutions.append((height, int(solution['fun']), len(solution['hashes']), ';'.join(solution['hashes'])))
                if len(solutions)%10 == 0:
                    # create dataframe
                    df = pd.DataFrame(solutions)
                    # set column names
                    df.columns = ['height', 'opt', 'tx_number', 'hashes_semicolon_separated']
                    # write to csv
                    df.to_csv('data/solutions.csv', index=False)
                    # print(height, int(solution['fun']))
                    print(f"Solved {len(solutions)}/{len(heights)} problems")

            except Exception as e:
                print(e)

    if len(solutions) > 0:
        # create dataframe
        df = pd.DataFrame(solutions)
        # set column names
        df.columns = ['height', 'opt', 'tx_number', 'hashes_semicolon_separated']
        # write to csv
        df.to_csv('data/solutions.csv', index=False)
        print(height, int(solution['fun']))
    
    print('Done!')

