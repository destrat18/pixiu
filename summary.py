import subprocess, os, time, json, random, db, generate_graph
import logging, re
import pandas as pd
from blockfrost import BlockFrostApi, ApiError, ApiUrls
from ast import literal_eval
import argparse, requests
from pandarallel import pandarallel
import tarfile

pandarallel.initialize(nb_workers=6)

import os

logging.basicConfig(level=logging.INFO)

# Please add web3 url's here
PROJECT_ID_LIST = [
    ]


DATA_PATH = "data/"

def fetch_pool_info(pool):
    try:
        pool_nodes_path = os.path.join(os.path.dirname(pool.pool_path), f"tx_pool_{pool.height}_nodes.csv")
        pool_edges_path = os.path.join(os.path.dirname(pool.pool_path), f"tx_pool_{pool.height}_edges.csv")
        
        if os.path.exists(pool_nodes_path):
            df_tx = pd.read_csv(pool_nodes_path)
            df_tx = df_tx[df_tx['size'].notna()].drop_duplicates('hash')
            
            df_tx['size'] = df_tx['size'].apply(int)
            df_tx['fees'] = df_tx['fees'].apply(int)

            return len(df_tx), df_tx['size'].sum(), df_tx['fees'].sum()
    except Exception as e:
        logging.error(e)
    return None, None, None

def get_real_block(height):
    try:
        api = BlockFrostApi(
            project_id=random.choice(PROJECT_ID_LIST),
            base_url=ApiUrls.mainnet.value,     
        )
        return api.block(int(str(height).split("-")[0])).to_dict()
    except Exception as e:
        logging.exception(e)
    
    return {}

def main(args):

    # Reading database credentials 
    block_csv_path = os.path.join(args.pool_dir, "block.csv")
    seen_height_list = []
    if os.path.exists(block_csv_path):
        seen_height_list = list(pd.read_csv(block_csv_path)['height'])
    
    logging.info(f'{len(seen_height_list)} blocks is Done!')

    block_list = []
    for f_name in os.listdir(args.pool_dir):
        if f_name.endswith('json'):
            pool_path = os.path.join(args.pool_dir, f_name) 
            with open(pool_path, 'r') as f:
                try:
                    pool = json.load(f)
                    pool_height = list(re.finditer(r"\d+(-\d)*", f_name, re.MULTILINE))[0].group()
                    
                    if pool_height not in seen_height_list:

                        block_list.append({
                            'hash': pool['block']['poolId'],
                            'height': pool_height,
                            'size': pool['block']['size'],
                            'fees': sum([tx['fee']['ada']['lovelace'] for tx in pool['block']['txs']]),
                            "pool_path": pool_path
                        })
                        if len(block_list) % 10 == 0:
                            logging.info(f"Processed: {len(block_list)}");  
                except:
                    pass
        
    df_block = pd.DataFrame(block_list)
    if len(df_block) > 0:    
        df_block = df_block.sort_values('height')
        df_block['bf'] = None 
        df_block['bf'] = df_block['height'].parallel_apply(get_real_block)

        df_block['pool_tx_count'], df_block['pool_size'], df_block['pool_fees'] = zip(*df_block.parallel_apply(fetch_pool_info, axis=1))
            
        # Update state csv
        df_block.to_csv(
            block_csv_path,
            index=False,
            header=not os.path.exists(block_csv_path),
            mode='a'
        )

    logging.info(f'Done! Added {len(df_block)}')



if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Pixiu: Optimal Block Production Revenues on Cardano')
    parser.add_argument('--pgpass', type=str, help='Path to postgresql pgpass for authentication', default='data/configuration/pgpass')
    parser.add_argument('--pool-dir', type=str, help='Path to data dir', default='data/cardano_tx_pool')
    args = parser.parse_args()

    main(args)
