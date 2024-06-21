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

def main(args):
    
    # Reading database credentials 
    with open(args.pgpass, 'r') as f:
        host, port, database, user, password = f.read().split(':')
    db_url = 'postgresql://{}@{}:{}/{}'.format(user, host, port, database)

    # Get list of all tx_pool csvs in the Data dir
    logging.info(f'Dumping {args.end - args.start} blocks')

    logging.info(db.get_latest_block(db_url=db_url))

    df_block = db.get_block_by_height_range(args.start, args.end, db_url=db_url)

    print(df_block)

    df_block.to_csv(
        args.output,
        index=False
    )
    logging.info(f'Done!')





if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='I super hate my life')
    parser.add_argument('--pgpass', type=str, help='Path to postgresql pgpass for authentication', default='data/configuration/pgpass')
    parser.add_argument('--start', type=int, help='first block number')
    parser.add_argument('--end', type=int, help='last block number')
    parser.add_argument('--output', type=str, help='path to output')
    
    
    args = parser.parse_args()

    main(args)
