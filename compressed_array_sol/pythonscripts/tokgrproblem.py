# takes two files: tx_pool_{}_nodes.csv and tx_pool_{}_edges.csv and creates a .kgr -- knapsack graph problem file

import os 
import sys
import pandas as pd


def parse_to_dict(path_to_nodes, path_to_edges):
    edges_df = pd.read_csv(path_to_edges)
    nodes_df = pd.read_csv(path_to_nodes)

    # nodes_df: hash,fees,size
    # edges_df: source,target,type

    # sort lines by hash:
    nodes_df.sort_values(by=['hash'], inplace=True)
    nodes_df.reset_index(drop=True, inplace=True)

    # print(nodes_df)

    hash_to_idx = {}
    idx_to_hash = {}
    
    idx_to_volume = {}
    idx_to_cost = {}
    edges = [] # (source, target, type)


    # create a dictionary of hash to index:
    for i, row in nodes_df.iterrows():
        hash_to_idx[row['hash']] = i + 1
        idx_to_hash[i + 1] = row['hash']
        idx_to_cost[i + 1] = int(row['fees'])
        idx_to_volume[i + 1] = int(row['size'])

    # create a list of edges:
    for i, row in edges_df.iterrows():
        edges.append((hash_to_idx[row['source']], hash_to_idx[row['target']], row['type']))

    problem = {
        "hash_to_idx": hash_to_idx,
        "idx_to_hash": idx_to_hash,
        "idx_to_hash": idx_to_hash,
        "idx_to_volume": idx_to_volume,
        "idx_to_cost": idx_to_cost,
        "edges": edges
    }

    return problem


def dict_to_string(parsed_info, max_capacity=90112):
    """
    form a file string as follows:
    num_verticies num_edges
    # n numbers indicating the cost of each vertex
    # n numbers indicating the volume of each vertex
    # m lines of edges, each line contains three numbers: source, target, type
    # n lines index to hash
    """

    ans = ""
    ans += f"{len(parsed_info['idx_to_hash'])} {len(parsed_info['edges'])}\n"
    for i in range(1, len(parsed_info['idx_to_hash']) + 1):
        ans += f"{parsed_info['idx_to_cost'][i]} "
    ans += "\n"

    for i in range(1, len(parsed_info['idx_to_hash']) + 1):
        ans += f"{parsed_info['idx_to_volume'][i]} "
    ans += "\n"

    for edge in parsed_info['edges']:
        ans += f"{edge[0]} {edge[1]} {edge[2]}\n"

    for i in range(1, len(parsed_info['idx_to_hash']) + 1):
        ans += f"{parsed_info['idx_to_hash'][i]}\n"

    ans += f"{max_capacity}\n"

    return ans



def to_kgr(path_to_nodes, path_to_edges, output_path):
    parsed_info = parse_to_dict(path_to_nodes, path_to_edges)
    # print(parsed_info)
    file_string = dict_to_string(parsed_info)
    with open(output_path, 'w') as f:
        f.write(file_string)