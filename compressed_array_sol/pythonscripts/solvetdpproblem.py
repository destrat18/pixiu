import os 
import sys
import subprocess
import signal
# import popen



def solve_tdp(path_to_problem, write_path, path_to_solver, time_limit=0.1):
    """ run the solver path_to_sover with stdin as path_to_problem and stdout as write_path
    ofter time_limit seconds, send SIGTERM to the solver """

    # print(path_to_problem)
    # print(write_path)
    # print(path_to_solver)
    # print(time_limit)

    with open(path_to_problem, 'r') as f:
        problem = f.read()
    
    # print(problem)

    p = subprocess.Popen([path_to_solver], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    p.stdin.write(problem.encode())
    p.stdin.close()
    
    try:
        p.wait(timeout=time_limit)
    except subprocess.TimeoutExpired:
        # send SIGTERM
        p.send_signal(signal.SIGTERM)
        p.wait()
    
    with open(write_path, 'w') as f:
        text_to_write = p.stdout.read().decode()
        if (text_to_write.strip() == ""):
            raise Exception("Empty output")
        
        text_to_write_copy = text_to_write
        # get first line
        tdp_value = int(text_to_write_copy.split('\n')[0])
        f.write(text_to_write)
        return tdp_value
