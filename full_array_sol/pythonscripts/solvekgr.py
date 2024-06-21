import os 
import sys
import subprocess
import signal

def solve_using_dynamic_programming(problem_path_to_kgr, path_to_tdp_solution, path_to_our_solution, PATH_TO_OUR_SOLVER):
    """out solver takes three arguments: path_to_kgr, path_to_tdp_solution, path_to_our_solution"""
    
    # check if path_to_tdp_solution exists
    if not os.path.exists(path_to_tdp_solution):
        # print to stderr that problem_path_to_kgr skipped
        print(f"problem_path_to_kgr skipped: {problem_path_to_kgr}", file=sys.stderr)
        return
    
    p = subprocess.Popen([PATH_TO_OUR_SOLVER, problem_path_to_kgr, path_to_tdp_solution, path_to_our_solution])
    p.wait()

    if p.returncode != 0:
        raise Exception(f"Solver failed with return code {p.returncode}")
