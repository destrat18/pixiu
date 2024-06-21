import os
import sys

from pythonscripts.tokgrproblem import to_kgr
from pythonscripts.kgrtotdpproblem import kgr_to_tdp_problem
from pythonscripts.solvetdpproblem import solve_tdp
from pythonscripts.solvekgr import solve_using_dynamic_programming
import concurrent.futures
from pathlib import Path
import pandas as pd
import time



PATH_TO_PROCESSED = 'processed/'
# PATH_TO_LIST_OF_PROBLEMS = 'listofheights'
PATH_TO_TEMP = 'temptdpworingdir/'
PATH_TO_SOL = 'solutions/'
PATH_TO_TDP_SOLVER = 'flow-cutter-tdp/flow_cutter_pace20'
PATH_TO_OUR_SOLVER = './src/run'

CSV_HEADER = ['problem', 'tdp_value', 'opt', 'time_preprocessing', 'time_our_solver', 'time_total', 'transactions']


TD_THRESHOLD = 16

def path_to_nodes(height):
    return os.path.join(PATH_TO_PROCESSED, f"tx_pool_{height}_nodes.csv")

def path_to_edges(height):
    return os.path.join(PATH_TO_PROCESSED, f"tx_pool_{height}_edges.csv")


def path_to_kgr(height):
    return os.path.join(PATH_TO_TEMP, f"problem_{height}.kgr")


def path_to_tdp_problem(PATH_TO_TEMP, height):
    return os.path.join(PATH_TO_TEMP, f"problem_{height}.tdp")


def get_path_to_tdp_solution(PATH_TO_TEMP, height):
    return os.path.join(PATH_TO_TEMP, f"solution_{height}.tdp")

def get_path_to_our_solution(PATH_TO_SOL, height):
    return os.path.join(PATH_TO_SOL, f"solution_{height}.solution")

def all_problems():
    all_files = os.listdir(PATH_TO_PROCESSED)
    nodes_files = [f for f in all_files if f.endswith('_nodes.csv')]
    heights = [f.split('_')[2] for f in nodes_files]
    return heights


def prepare_directory():
    if not os.path.exists(PATH_TO_TEMP):
        os.mkdir(PATH_TO_TEMP)

    if not os.path.exists(PATH_TO_SOL):
        os.mkdir(PATH_TO_SOL)

def solve_problem(problem):
    # print("Solving problem: ", problem)
    path_to_node = path_to_nodes(problem)
    path_to_edge = path_to_edges(problem)
    problem_path_to_kgr = path_to_kgr(problem)
    
    time_start = time.time()

    # converting to kgr
    to_kgr(path_to_node, path_to_edge, problem_path_to_kgr)

    # converting to tdp instance
    problem_path_to_tdp = path_to_tdp_problem(PATH_TO_TEMP, problem)
    kgr_to_tdp_problem(problem_path_to_kgr, problem_path_to_tdp)

    # solving tdp instance
    path_to_tdp_solution = get_path_to_tdp_solution(PATH_TO_TEMP, problem)
    
    try:
        tdp_value = solve_tdp(problem_path_to_tdp, path_to_tdp_solution, PATH_TO_TDP_SOLVER)
        tdp_value = int(tdp_value)
        if tdp_value > TD_THRESHOLD:
            print(f"problem_path_to_kgr: {problem_path_to_kgr} has too high tree depth: {tdp_value}, skipped", file=sys.stderr)
    except Exception as e:
        print(f"problem_path_to_kgr skipped: {problem_path_to_kgr}", file=sys.stderr)
        raise e

    # solving our instance
    try:
        time_preprocessing_finished = time.time()

        path_to_our_solution = get_path_to_our_solution(PATH_TO_SOL, problem)
        solve_using_dynamic_programming(problem_path_to_kgr, path_to_tdp_solution, path_to_our_solution, PATH_TO_OUR_SOLVER)

        time_our_solver_finished = time.time()


        # read solution 
        with open(path_to_our_solution, 'r') as f:
            our_solution = f.read()
        # get the opt which is the second line
        opt = int(our_solution.split('\n')[1]) 

        tx_to_include = our_solution.split('\n')[4:]
        tx_to_include = [tx for tx in tx_to_include if tx.strip() != '']

        transactions = ";".join(tx_to_include)


        return (problem, tdp_value, opt, time_preprocessing_finished - time_start, time_our_solver_finished - time_preprocessing_finished, time_our_solver_finished - time_start, transactions)
    except Exception as e:
        print(f"solver failure for problem_path_to_kgr: {problem_path_to_kgr}", file=sys.stderr)
        raise e

if __name__ == "__main__":

    # TODO make it argument when we have time
    # Read processed dir and result dir from argv
    if len(sys.argv) > 2:
        PATH_TO_PROCESSED = sys.argv[1] 
        output_dir = sys.argv[2]

    os.makedirs(output_dir, exist_ok=True)
    output_path = os.path.join(output_dir, 'knapsack.csv')

    prepare_directory()
    problems_list = all_problems()

    cnt = 0

    try:
        df = pd.read_csv(output_path)
        seen_problem = list(df.problem)
        problems_list = list(set(problems_list)-set(seen_problem))
    except Exception as e:
        print(e)
    table = []

    with concurrent.futures.ThreadPoolExecutor(max_workers=10) as executor:
        
        future_to_height = {}
        for problem in problems_list:
            future_to_height[executor.submit(solve_problem, problem)] =  problem


        for future in concurrent.futures.as_completed(future_to_height):
            height = future_to_height[future]
            try:
                table.append(future.result())
                cnt += 1
                # exit(0)
                if (cnt % 10 == 0):
                    print(f"Solved {cnt}/{len(problems_list)} problems")
                    # Save partially since all of them might take too long
                    df = pd.DataFrame(table)
                    df.columns = CSV_HEADER
                    df.to_csv(
                        output_path, 
                        index=False,
                        mode='a',
                        header=not os.path.exists(output_path))
                    table = []
            except Exception as e:
                print(e)

    if len(table):
        df = pd.DataFrame(table)
        df.columns = CSV_HEADER
        df.to_csv(
            output_path, 
            index=False,
            mode='a',
            header=not os.path.exists(output_path))
    
    print('Done!')

