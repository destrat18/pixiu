1. install boost `sudo apt-get install libboost-all-dev`

2. Place all processed files into `./processed` folder

3. Build flowcutter `cd flow-cutter-tdp/ && chmod +x build.sh && ./build.sh && cd ..`

4. Build solver: `make`

5. Install python dependencies: `pip3 install -r requirements.txt`

6. Run `python3 solveeverything.py` to run all experiments

The solutions and timings will appear in `solveeverything_report.csv` file. Additionally, for each instance, a solution file `solutions/solution_<height>.solution` will be created.