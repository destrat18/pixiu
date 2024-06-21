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

logging.basicConfig(level=logging.INFO)

DATA_PATH = "data/"
PROCESSED_DIR = os.path.join(DATA_PATH, "processed")


"""
Fortunately, Hydra already has to deal with double-spend conflicts of this kind (although in the naive protocol this results in a decommit). This proposal simply introduces a new kind of conflict, a reference-spend conflict. Reference-spend conflicts should be handled by the same mechanism that is used to handle double-spend conflicts.
https://developers.cardano.org/docs/governance/cardano-improvement-proposals/CIP-0031

The user specifies the UTxO entries containing funds sufficient to cover a percentage
(usually 100 or more) of the total transaction fee. These inputs are only collected in the
case of script validation failure, and are called collateral inputs. In the case of script validation
success, the fee specified in the fee field of the transaction is collected, but the collateral is
not.
https://github.com/input-output-hk/cardano-ledger/releases/latest/download/alonzo-ledger.pdf, page 4

The new field collateral is used to specify the collateral inputs that are collected (into the
fee pot) to cover a percentage of transaction fees (usually 100 percent or more) in case the
transaction contains failing phase-2 scripts. They are not collected otherwise. The purpose of
the collateral is to cover the resource use costs incurred by block producers running scripts
that do not validate.
Collateral inputs behave like regular inputs, except that they must be VKey locked and
can only contain Ada. See 4.
It is permitted to use the same inputs as both collateral and a regular inputs, as exactly
one set of inputs is ever collected: collateral ones in the case of script failure, and regular
inputs in the case when all scripts validate.
https://github.com/input-output-hk/cardano-ledger/releases/latest/download/alonzo-ledger.pdf, page 11


Transaction integrity and charging for failing scripts. If a transaction contains a failing
script, the only change to the ledger that is made is that the collateral is collected into the fee pot.
It is important to note here that it can be the case that the only signatures on a transaction are
those of the keys for the collateral UTxOs (but this must include at least one signature).
The implication of this is that the collateral key owners may be the only users that attest to
the integrity of the data in the body of the transaction. These same key owners, however, are also
the only users who stand to lose money if the transaction is modified in some way that results
in phase-2 failure. Transactions with the same body will necessarily have the same outcome of
phase-2 script validation (we give the details in the deterministic script validation property,
https://github.com/input-output-hk/cardano-ledger/releases/latest/download/alonzo-ledger.pdf, page 18

Figure 14 separates the case where all the scripts in a transaction successfully validate from the
case where there is one or more that does not. These two cases are distinguished by the use
of the IsValid tag. Besides collateral collection, no side effects should occur when processing
a transaction that contains a script that does not validate. That is, no delegation or pool state
updates, update proposals, or any other observable state change, should be applied. The UTxO
rule is still applied, however, as this is where the correctness of the validation tag is verified,
and where collateral inputs are collected.
https://github.com/input-output-hk/cardano-ledger/releases/latest/download/alonzo-ledger.pdf, page 25


"""

# Find confilicts
def find_confilicts(df_tx):
    confilict_list = set()
    for i, tx_i in df_tx.iterrows():
        for j, tx_j in df_tx.iloc[i:].iterrows():
            if tx_i['valid_contract'] == True and tx_j['valid_contract'] == True: 
                if i != j:
                    for inp_i in tx_i.inputs:
                        for inp_j in tx_j.inputs:
                            # Because the transaction is exectued successfully, we should only spend collateral=False inputs
                            if inp_i['collateral'] == False and inp_j['collateral'] == False:            
                                # Inputs contain tx_hash and output_index, which indicate which output of which tx they are dependent on. 
                                if inp_i['tx_hash'] == inp_j['tx_hash'] and inp_i['output_index'] == inp_j['output_index']:
                                    # If on of the inputs are using this output as a reference, then we don't have any problem
                                    if inp_i['reference'] == False and inp_j['reference'] == False: 
                                        confilict_list.add((tx_i['hash'], tx_j['hash'], 'c'))       
            else:
                raise Exception(f'Found invalid contract case. Please study this: {tx_i}, {tx_j}')
    return confilict_list


# Find dependencies
def find_dependencies(df_tx):
    dependency_list = set()
    for i, tx_i in df_tx.iterrows():
        for j, tx_j in df_tx.iterrows():
            if tx_j['valid_contract'] == True:
                for inp_j in tx_j['inputs']:
                    # tx_j depends on tx_i when one of tx_j inputs contains tx_i hash. It does not mater
                    # if it is referencing it or not.
                    # We need to check refrences for every input even colletrals.
                    if inp_j['tx_hash'] == tx_i['hash']:
                        dependency_list.add((tx_i['hash'], tx_j['hash'], 'd'))
            else:
                raise Exception(f'Found invalid contract case. Please study this: {tx_i}, {tx_j}')
    return dependency_list


def json_to_df(pool):
    tx_list = []
    for mem in pool:
        tx = {
            'hash': mem['id'],
            'fees': mem['fee']['ada']['lovelace'],
            'size': len(mem['cbor'])/2-1,
            'inputs': [],
            'valid_contract': True
        }

        # WE DON't CARE ABOUT OUTPUTS

        # Normal Inputs
        for inp in mem['inputs']:
            tx['inputs'].append(
                {
                    'tx_hash': inp['transaction']['id'], 
                    'output_index': inp['index'], 
                    'collateral': False, 
                    'reference': False
                    }
            )

        # Collaterals Inputs
        if "collaterals" in mem:
            for inp in mem['collaterals']:
                tx['inputs'].append(
                    {
                        'tx_hash': inp['transaction']['id'], 
                        'output_index': inp['index'], 
                        'collateral': True, 
                        'reference': False
                        }
                )

        # Refrence inputs
        if "references" in mem:
            for inp in mem['references']:
                tx['inputs'].append(
                    {
                        'tx_hash': inp['transaction']['id'], 
                        'output_index': inp['index'], 
                        'collateral': False, 
                        'reference': True
                        }
                )

        tx_list.append(tx)


    return pd.DataFrame(tx_list)

def generate_graph_for_pool_name(block_number, pool_path):
    try:
        
        if pool_path.endswith('.csv'):
            df_tx = pd.read_csv(pool_path)
            df_tx.inputs = df_tx.inputs.apply(str).apply(literal_eval)
            df_tx.outputs = df_tx.outputs.apply(str).apply(literal_eval)
            assert(len(find_confilicts(df_tx[df_tx['source']=="block"]))==0)
        
        elif pool_path.endswith(".json"):
            
            with open(pool_path, 'r') as f:
                pool = json.load(f)
            
            df_tx = pd.concat([json_to_df(pool['memPoolState']),json_to_df(pool['block']['txs'])])
            assert(len(find_confilicts(json_to_df(pool['block']['txs'])))==0)
        
        assert(len(df_tx)>0)

        df_tx['size'] = df_tx['size'].apply(int)
        df_tx['fees'] = df_tx['fees'].apply(int)

        # To be sure that we don't find any conflict within a block
        df_tx = df_tx[df_tx['size'].notna()].drop_duplicates('hash')
                
        # # calculate confilicts and dependecies
        confilicts = find_confilicts(df_tx) # find confilicts
        dependencies = find_dependencies(df_tx) # find dependencies
        df_c = pd.DataFrame(confilicts, columns=['source', 'target', 'type'])
        df_d = pd.DataFrame(dependencies, columns=['source', 'target', 'type'])
        df_dc = pd.concat([df_c, df_d], ignore_index=True)
        
        # DC Graph
        # Store edges for DC Graph
        df_dc.to_csv(
            os.path.join(os.path.dirname(pool_path), f"tx_pool_{block_number}_edges.csv"), 
            index=False)
        # Store nodes for DC Graph
        df_tx[['hash', 'fees', 'size']].to_csv(
            os.path.join(os.path.dirname(pool_path), f"tx_pool_{block_number}_nodes.csv"), 
            index=False)

        logging.info(
            "Block {}: {} txs, size: {}, c: {}, d: {}".format(
                pool_path,
                len(df_tx),
                df_tx['size'].sum(),
                len(df_c),
                len(df_d)
                )
            )

    except Exception as e:
        logging.error(f"{pool_path}: {e}")

def main(data_dir):
    pool_list = []
    
    regex = r"tx_pool_\d+.csv|\d+(-\d)*.json"
    for f_name in os.listdir(data_dir):
        matches = list(re.finditer(regex, f_name, re.MULTILINE))
        if not len(matches):
            continue
        pool_height = list(re.finditer(r"\d+(-\d)*", f_name, re.MULTILINE))[0].group()
        
        # if f_name.startswith('tx_pool') and f_name.endswith('.csv') and not f_name.endswith('_nodes.csv') and not f_name.endswith('_edges.csv'):
        if not os.path.exists(os.path.join(data_dir, f"tx_pool_{pool_height}_edges.csv")):
            pool_list.append({
                'block_number': pool_height,
                "pool_file": os.path.join(data_dir,f_name)
            })

    logging.info(f'Generating nodes and edges for {len(pool_list)} pools')
    if len(pool_list):
        pd.DataFrame(pool_list).sort_values('block_number').parallel_apply(
            lambda row: generate_graph_for_pool_name(row.block_number, row.pool_file),
            axis=1)
        


if __name__ == "__main__":    
    
    parser = argparse.ArgumentParser(description='Pixiu: Optimal Block Production Revenues on Cardano')
    parser.add_argument('--pool-dir', type=str, help='Path to data dir', default='data/cardano_tx_pool')
    args = parser.parse_args()
    
    try:
        main(args.pool_dir)
    except Exception as e:
        logging.exception(e)
    