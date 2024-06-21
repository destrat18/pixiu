import subprocess, os, time, json, random
import logging
import pandas as pd
import concurrent.futures
import argparse, requests
from ast import literal_eval
from contextlib import suppress
import db
import os
from pandarallel import pandarallel

logging.basicConfig(level=logging.INFO)
pandarallel.initialize(nb_workers=2)




def get_bf_tx(tx_hash, project_id):
    headers = {
        'Accept': 'application/json',
        'project_id': project_id,
    }

    response = requests.get(f'https://cardano-mainnet.blockfrost.io/api/v0/txs/{tx_hash}', headers=headers)

    return response

def get_bf_tx_utxos(tx_hash, project_id):
    headers = {
        'Accept': 'application/json',
        'project_id': project_id,
    }

    response = requests.get(f'https://cardano-mainnet.blockfrost.io/api/v0/txs/{tx_hash}/utxos', headers=headers)

    return response



def get_tx(tx_hash, block_height, project_id):
    tx_response = get_bf_tx(tx_hash, project_id)
    tx_response.raise_for_status()
    
    
    tx_response = tx_response.json()
    if tx_response['block_height'] < block_height:
        return None
    
    tx_utxos_response = get_bf_tx_utxos(tx_hash, project_id)
    tx_utxos_response.raise_for_status()
    tx_utxos_response = tx_utxos_response.json()

    return {
        'hash': tx_hash,
        'tx': tx_response,
        'inputs': tx_utxos_response['inputs'] if 'inputs' in tx_utxos_response else None,
        'outputs': tx_utxos_response['outputs'] if 'outputs' in tx_utxos_response else None,
        'redeemers': None
    
    }

def raw_to_df_tx(df_tx_raw):
    processed_tx_list = {}
    for i, row in df_tx_raw[df_tx_raw['tx'].notna()].iterrows():
        try:
            tx = {}
            tx['hash'] = row.tx['hash']
            tx['fees'] = row.tx['fees']
            tx['size'] = row.tx['size']
            tx['deposit'] = row.tx['deposit']
            tx['valid_contract'] = row.tx['valid_contract']
            tx['invalid_before'] = row.tx['invalid_before']
            tx['invalid_hereafter'] = row.tx['invalid_hereafter']

            # Add inputs
            tx['inputs'] = []
            for i in row.inputs:
                tx['inputs'].append({
                    'tx_hash': i['tx_hash'],
                    'output_index': i['output_index'],
                    'collateral': i['collateral'],
                    'reference': i['reference'],
                    
                })

            # Add outputs
            tx['outputs'] = []
            for o in row.outputs:
                tx['outputs'].append({
                    'address': o['address'],
                    'output_index': o['output_index'],
                    'collateral': o['collateral'],
                    'reference_script_hash': o['reference_script_hash']
                })

            processed_tx_list[row.tx['hash']]=tx
        except Exception as e:
            raise e
    
    return pd.DataFrame.from_dict(processed_tx_list, orient='index')


def database_to_df_tx(
        df_tx,
        # Inputs
        df_tx_in,
        df_tx_collateral_tx_in,
        df_tx_reference_tx_in,
        # Outputs
        df_tx_out,
        df_tx_collateral_tx_out
    ):

    # I'm going crazy
    
    processed_tx_list = {}
    for _, row in df_tx.iterrows():
        tx = {}
        tx['hash'] = row['hash']
        tx['fees'] = row['fee']
        tx['size'] = row['size']
        tx['deposit'] = row['deposit']
        tx['valid_contract'] = row['valid_contract']
        tx['invalid_before'] = row['invalid_before']
        tx['invalid_hereafter'] = row['invalid_hereafter']


        tx['inputs'] = []
        for i, tx_i in df_tx_in[df_tx_in.tx_in_hash == row['hash']].iterrows():
            tx['inputs'].append({
                'tx_hash': tx_i['tx_out_hash'],
                'output_index': tx_i['tx_out_index'],
                'collateral': False,
                'reference': False,
                
            })
        # Add refrence values
        for i, tx_i in df_tx_reference_tx_in[df_tx_reference_tx_in.tx_in_hash == row['hash']].iterrows():
            tx['inputs'].append({
                'tx_hash': tx_i['tx_out_hash'],
                'output_index': tx_i['tx_out_index'],
                'collateral': False,
                'reference': True,
                
            })
        # Add coll values
        for i, tx_i in df_tx_collateral_tx_in[df_tx_collateral_tx_in.tx_in_hash == row['hash']].iterrows():
            tx['inputs'].append({
                'tx_hash': tx_i['tx_out_hash'],
                'output_index': tx_i['tx_out_index'],
                'collateral': True,
                'reference': False,
                
            })
        
        

        tx['outputs'] = []
        for i, tx_o in df_tx_out[df_tx_out.tx_hash == row['hash']].iterrows():
            tx['outputs'].append({
                'address': tx_o['address'],
                'output_index': tx_o['index'],
                'collateral': False,
                # 'reference_script_hash': tx_o['reference_script_id']
            })
        for i, tx_o in df_tx_collateral_tx_out[df_tx_collateral_tx_out.tx_hash == row['hash']].iterrows():
            tx['outputs'].append({
                'address': tx_o['address'],
                'output_index': tx_o['index'],
                'collateral': True,
                # 'reference_script_hash': tx_o['reference_script_id']
            })

        processed_tx_list[tx['hash']]=tx

    return pd.DataFrame.from_dict(processed_tx_list, orient='index')

def is_double_spending(row, block_height, db_url):
    for inp in row.inputs:
        if inp['collateral']==False:
            df = db.has_conflict(
                block_no=block_height,
                tx_hash=inp['tx_hash'],
                output_index=inp['output_index'],
                db_url=db_url
            )
            if len(df) > 0:
                df = df.map(lambda x: x.hex() if type(x) == memoryview else x)
                return df.to_dict('records')
    return []    


def process_tx_pool(block_height, data_dir, db_url):

    try:
        start_time = time.time()

        # Add transactions from raw pool
        df_bf_pool = pd.read_csv(
            os.path.join(args.data_dir, 'bf_tx_pool', 'bf_tx_pool.csv')
        )
        
        df_bf_total_tx = pd.read_csv(
            os.path.join(args.data_dir, 'bf_tx_pool', 'tx.csv')
        )

        df_tx_bf = df_bf_total_tx[df_bf_total_tx['hash'].isin(df_bf_pool[df_bf_pool.block_height==block_height]['hash'])].drop_duplicates('hash')
        del df_bf_total_tx, df_bf_pool
        df_tx_bf['tx'] = df_tx_bf['tx'].apply(lambda x: literal_eval(x) if pd.notnull(x) else None)
        df_tx_bf['inputs'] = df_tx_bf['inputs'].apply(lambda x: literal_eval(x) if pd.notnull(x) else None)
        df_tx_bf['outputs'] = df_tx_bf['outputs'].apply(lambda x: literal_eval(x) if pd.notnull(x) else None)
        
        ################ RAW ################

        # Add from txs from raw transaction
        df_tx_bf_processed = raw_to_df_tx(df_tx_bf)
        df_tx_bf_processed['source'] = 'blockfrost'

        ################ TARGET BLOCK  ################

        # Add txs that are included in the target block
        block = db.get_block(block_height, db_url)
        df_block_dict = {
            'df_tx': db.get_transactions_by_block_id(block['id'], db_url),
            
            #Input Queries
            "df_tx_in": db.get_transactions_tx_in_by_block_id(block['id'], db_url),
            'df_tx_collateral_tx_in': db.get_transactions_collateral_tx_in_by_block_id(block['id'], db_url),
            'df_tx_reference_tx_in': db.get_transactions_reference_tx_in_by_block_id(block['id'], db_url),
            
            # Output queries
            "df_tx_out": db.get_transactions_tx_out_by_block_id(block['id'], db_url),
            'df_tx_collateral_tx_out': db.get_transactions_collateral_tx_out_by_block_id(block['id'], db_url),
        }

        # Fix memoryviews
        for df_name in df_block_dict:
            df_block_dict[df_name] = df_block_dict[df_name].map(lambda x: x.hex() if type(x) == memoryview else x)

        df_tx_block_processed = database_to_df_tx(**df_block_dict)
        df_tx_block_processed['source'] = 'block'
 

        ########### Put it all together WAY ###########
        
        df_tx_processed = pd.concat([df_tx_bf_processed, df_tx_block_processed])
        
        # Remove duplicates
        # df_tx_processed = df_tx_processed.drop_duplicates('hash', keep='first')

        # Remove wrong range
        # block['slot_no'] = int(block['slot_no'])
        # df_tx_processed['is_valid_range'] = df_tx_processed['invalid_before'].apply(lambda x: True if pd.isnull(x) else int(x)<=block['slot_no']) & \
        #     df_tx_processed['invalid_hereafter'].apply(lambda x: True if pd.isnull(x) else int(x)>block['slot_no'])
        # invalid_range_no = len(df_tx_processed[df_tx_processed['is_valid_range']==False])
        # assert(invalid_range_no > 0)
        # df_tx_processed = df_tx_processed[df_tx_processed['is_valid_range'] == True]

        # # Remove Double Spendings
        # df_tx_processed['conflict'] = df_tx_processed.apply(lambda row: is_double_spending(row, block_height, db_url),axis=1)
        # df_tx_processed['is_double_spending'] = df_tx_processed['conflict'].apply(lambda x: len(x)>0)
        # df_tx_processed.to_csv(
        #     f"data/temp/tx_pool_{block_height}.csv",
        #     index=False
        # )
        # double_spend_no = len(df_tx_processed[df_tx_processed['is_double_spending']==True])
        # df_tx_processed = df_tx_processed[df_tx_processed['is_double_spending']==False]

        # To be double sure
        assert(set(df_block_dict['df_tx']['hash']).issubset(set(df_tx_processed[df_tx_processed['size'].notna()]['hash'])))

        target_pool_file_name = f"tx_pool_{block_height}.csv"

        # Save processed tx in the processed dir in the data
        df_tx_processed.to_csv(os.path.join(data_dir, "bf_tx_pool" ,target_pool_file_name), index=False)

        logging.info(
            "Block {}: {} TXs, {}/90112 bytes, Raw Pool Size: {}, Invalid Contracts: {}, Sources: {}, T: {:.2f}".format(
                block_height,
                len(df_tx_processed[df_tx_processed['size'].notna()]),
                int(df_tx_processed[df_tx_processed['size'].notna()]['size'].sum()),
                len(df_tx_bf),
                list(df_tx_processed[df_tx_processed['valid_contract']==False]['hash']),
                df_tx_processed['source'].value_counts().to_dict(),
                time.time()-start_time
                )
            )



        return True
    
    except Exception as e:
        logging.error(f"{block_height}:{str(e)}")
        logging.exception(e)
    
    return False

def main(args):
    
    # Reading database credentials 
    with open(args.pgpass, 'r') as f:
        host, port, database, user, password = f.read().split(':')
    db_url = 'postgresql://{}@{}:{}/{}'.format(user, host, port, database)

    # Get list of all tx_pool csvs in the Data dir
    df_pool = pd.read_csv(
        os.path.join(args.data_dir, 'bf_tx_pool', 'bf_tx_pool.csv')
    )
    
    if args.block:
        df_pool = df_pool[df_pool.block_height == int(args.block)]

    df_pool = df_pool.drop_duplicates('block_height')
    df_pool['is_processed'] = df_pool.block_height.apply(
        lambda block_height: os.path.exists(
            os.path.join(args.data_dir, "bf_tx_pool" ,f"tx_pool_{block_height}.csv")
            )
        )

    df_pool = df_pool[df_pool['is_processed']==False]

    logging.info(f'Processing {len(df_pool.block_height.unique())} pools')
    if len(df_pool) == 0:
        return
    
    df_pool.drop_duplicates('block_height').sort_values('block_height').block_height.parallel_apply(lambda block_height: process_tx_pool(
        block_height=block_height, data_dir=args.data_dir, db_url=db_url))





if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Pixiu: Optimal Block Production Revenues on Cardano')
    parser.add_argument('--pgpass', type=str, help='Path to postgresql pgpass for authentication', default='data/configuration/pgpass')
    parser.add_argument('--data-dir', type=str, help='Path to data dir', default='data')
    parser.add_argument('--block', type=int, help='Prepare for specific block', default=None)
    args = parser.parse_args()

    while True:
        try:
            main(args)
        except Exception as e:  
            logging.exception(e)
        
        time.sleep(60*5)
