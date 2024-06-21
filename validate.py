import subprocess, os, time, json, random
import logging
import pandas as pd
from blockfrost import BlockFrostApi, ApiError, ApiUrls
from ast import literal_eval
from pandarallel import pandarallel
import argparse, requests
import re

pandarallel.initialize()

import os

MAX_BLOCK_SIZE = 90112

logging.basicConfig(level=logging.INFO)
def validate_sol(sol, pool_dir):
    try:    
        tx_list = sol['transactions'].split(";")
        df_edge = pd.read_csv(os.path.join(pool_dir, f'tx_pool_{sol["problem"]}_edges.csv'))
        df_node = pd.read_csv(os.path.join(pool_dir, f'tx_pool_{sol["problem"]}_nodes.csv'))
        
        for i, e in df_edge.iterrows():
            # Block should not have any conflicts
            if e['type'] == 'c':
                if e['source'] in tx_list and e['target'] in tx_list:
                    raise Exception(f'Pool {sol["problem"]} has a conflit: {e}')
            # Block should follow dependencies
            elif  e['type'] == 'd':
                if not e['source'] in tx_list and e['target'] in tx_list:
                    raise Exception(f'Pool {sol["problem"]} does not follow a dependency: {e}')

        # Total size of transactions must be less than max block size
        if df_node[df_node['hash'].isin(tx_list)]['size'].sum() > MAX_BLOCK_SIZE:
            raise Exception(f'Pool {sol["problem"]} is bigger than max block size: {e}')

        return True
    except Exception as e:
        logging.exception(e)
    
    return False

def main(pool_dir, solution_path):
    df_sol = pd.read_csv(solution_path)
    df_sol['is_valid'] = df_sol.parallel_apply(lambda sol: validate_sol(sol, pool_dir), axis=1)

    logging.info(f"{sum(df_sol['is_valid']==False)}/{len(df_sol)} is invalid!")


if __name__ == "__main__":    
    
    parser = argparse.ArgumentParser(description='Pixiu: Optimal Block Production Revenues on Cardano')
    parser.add_argument('--pool-dir', type=str, help='Path to data dir', default='data/cardano_tx_pool')
    parser.add_argument('--solution-path', type=str, help='Path to solution', default='data/results/cardano/knapsack.csv')
    
    args = parser.parse_args()
    
    try:
        main(args.pool_dir, args.solution_path)
    except Exception as e:
        logging.exception(e)
    