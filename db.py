import pandas as pd
from sqlalchemy import create_engine
import logging

def get_latest_block(db_url):
    df = pd.read_sql_query("SELECT * FROM block ORDER BY block_no DESC NULLS LAST LIMIT 1", con=create_engine(db_url))
    return df.iloc[0]

###### BLOCK ID ######

def get_transactions_by_block_id(block_id, db_url):
    q = f"""
            SELECT 
                *
            FROM
                tx
            WHERE
                block_id={block_id}
        """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df


def get_transactions_tx_out_by_block_id(block_id, db_url):
    q = f"""
        SELECT
            tx_out.*, tx.hash as tx_hash
        FROM
            tx_out CROSS JOIN tx
        WHERE
            
            tx_out.tx_id = tx.id
            AND tx.block_id = {block_id}
        """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df


def get_transactions_collateral_tx_out_by_block_id(block_id, db_url):
    q = f"""
        SELECT
            collateral_tx_out.*, tx.hash as tx_hash
        FROM
            collateral_tx_out CROSS JOIN tx
        WHERE
            
            collateral_tx_out.tx_id = tx.id
            AND tx.block_id = {block_id}
        """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df

def get_transactions_tx_in_by_block_id(block_id, db_url):
    q = f"""
        SELECT
            tx_in.*, tx_i.hash as tx_in_hash, tx_o.hash as tx_out_hash
        FROM
            tx_in CROSS JOIN tx as tx_i CROSS JOIN tx as tx_o 
        WHERE
            tx_in.tx_in_id = tx_i.id
            AND tx_in.tx_out_id = tx_o.id
            AND tx_i.block_id = {block_id}

        """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df

def get_transactions_collateral_tx_in_by_block_id(block_id, db_url):
    q = f"""
        SELECT
            collateral_tx_in.*, tx_i.hash as tx_in_hash, tx_o.hash as tx_out_hash
        FROM
            collateral_tx_in CROSS JOIN tx as tx_i CROSS JOIN tx as tx_o 
        WHERE
            collateral_tx_in.tx_in_id = tx_i.id
            AND collateral_tx_in.tx_out_id = tx_o.id
            AND tx_i.block_id = {block_id}

        """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df

def get_transactions_reference_tx_in_by_block_id(block_id, db_url):
    q = f"""
        SELECT
            reference_tx_in.*, tx_i.hash as tx_in_hash, tx_o.hash as tx_out_hash
        FROM
            reference_tx_in CROSS JOIN tx as tx_i CROSS JOIN tx as tx_o 
        WHERE
            reference_tx_in.tx_in_id = tx_i.id
            AND reference_tx_in.tx_out_id = tx_o.id
            AND tx_i.block_id = {block_id}

        """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df


###### TX HASH LIST ######

def get_transactions_by_hash_list(tx_hash_list, db_url):
    ss = ""
    for h in tx_hash_list:
        ss = ss + ", " + "'"+"\\x"+str(h)+"'"
    ss = ss[1:]

    q = f"""
        SELECT
            tx.*, block.block_no
        FROM
            tx CROSS JOIN block
        WHERE
            tx.hash IN ({ss})
            AND tx.block_id = block.id

    """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df

#### FIX BELOW ####

def get_transactions_tx_out_by_hash_list(tx_hash_list, db_url):

    ss = ""
    for h in tx_hash_list:
        ss = ss + ", " + "'"+"\\x"+str(h)+"'"
    ss = ss[1:]

    q = f"""
        SELECT
            tx_out.*, tx.hash as tx_hash
        FROM
            tx_out CROSS JOIN tx
        WHERE
            
            tx_out.tx_id = tx.id
            AND tx.hash IN ({ss})
        """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df


def get_transactions_collateral_tx_out_by_hash_list(tx_hash_list, db_url):

    ss = ""
    for h in tx_hash_list:
        ss = ss + ", " + "'"+"\\x"+str(h)+"'"
    ss = ss[1:]

    q = f"""
        SELECT
            collateral_tx_out.*, tx.hash as tx_hash
        FROM
            collateral_tx_out CROSS JOIN tx
        WHERE
            
            collateral_tx_out.tx_id = tx.id
            AND tx.hash IN ({ss})
        """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df

def get_transactions_tx_in_by_hash_list(tx_hash_list, db_url):

    ss = ""
    for h in tx_hash_list:
        ss = ss + ", " + "'"+"\\x"+str(h)+"'"
    ss = ss[1:]

    q = f"""
        SELECT
            tx_in.*, tx_i.hash as tx_in_hash, tx_o.hash as tx_out_hash
        FROM
            tx_in CROSS JOIN tx as tx_i CROSS JOIN tx as tx_o 
        WHERE
            tx_in.tx_in_id = tx_i.id
            AND tx_in.tx_out_id = tx_o.id
            AND tx_i.hash IN ({ss})

        """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df

def get_transactions_collateral_tx_in_by_hash_list(tx_hash_list, db_url):

    ss = ""
    for h in tx_hash_list:
        ss = ss + ", " + "'"+"\\x"+str(h)+"'"
    ss = ss[1:]

    q = f"""
        SELECT
            collateral_tx_in.*, tx_i.hash as tx_in_hash, tx_o.hash as tx_out_hash
        FROM
            collateral_tx_in CROSS JOIN tx as tx_i CROSS JOIN tx as tx_o 
        WHERE
            collateral_tx_in.tx_in_id = tx_i.id
            AND collateral_tx_in.tx_out_id = tx_o.id
            AND tx_i.hash IN ({ss})

        """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df

def get_transactions_reference_tx_in_by_hash_list(tx_hash_list, db_url):

    ss = ""
    for h in tx_hash_list:
        ss = ss + ", " + "'"+"\\x"+str(h)+"'"
    ss = ss[1:]

    q = f"""
        SELECT
            reference_tx_in.*, tx_i.hash as tx_in_hash, tx_o.hash as tx_out_hash
        FROM
            reference_tx_in CROSS JOIN tx as tx_i CROSS JOIN tx as tx_o 
        WHERE
            reference_tx_in.tx_in_id = tx_i.id
            AND reference_tx_in.tx_out_id = tx_o.id
            AND tx_i.hash IN ({ss})

        """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df



def get_block(block_height, db_url):
    q = f"""
        SELECT 
            *
        FROM
            block
        WHERE
            block_no={block_height}
    """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df.iloc[0]

def get_block_by_height_range(block_height_start,block_height_end, db_url):
    q = f"""
        SELECT 
            block.*, block_fees.*
        FROM
            block CROSS JOIN (SELECT block_id, sum(fee) as fees FROM tx GROUP BY block_id) as block_fees
        WHERE
            block_no>={block_height_start}
            AND block_no<={block_height_end}
            AND block.id = block_fees.block_id
    """
    print(q)
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df


def has_conflict(block_no, tx_hash, output_index, db_url):

    tx_hash_s = "'"+"\\x"+str(tx_hash)+"'"

    q = f"""
        SELECT
            tx_in.*, tx_i.hash as tx_in_hash, tx_o.hash as tx_out_hash
        FROM
            tx_in CROSS JOIN tx as tx_i CROSS JOIN tx as tx_o CROSS JOIN block as tx_i_block
        WHERE
            tx_in.tx_in_id = tx_i.id
            AND tx_in.tx_out_id = tx_o.id
            AND tx_i.block_id=tx_i_block.id
            AND tx_o.hash = {tx_hash_s}
            AND tx_in.tx_out_index = {output_index}
            AND tx_i_block.block_no < {block_no}

        """
    df = pd.read_sql_query(q, con=create_engine(db_url))
    return df