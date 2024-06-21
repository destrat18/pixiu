import os
import sys  
import pandas as pd 
import seaborn as sns
import matplotlib.pyplot as plt
import math


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python plotwhiskers.py <path_to_csv>")
        sys.exit(1)

    path = sys.argv[1]
    if not os.path.exists(path):
        print("Error: file does not exist")
        sys.exit(1)
    # problem,tdp_value,opt,time_preprocessing,time_our_solver,time_total
    df = pd.read_csv(path)

    # apply log to time_total
    # df['time_total'] = df['time_total'].apply(lambda x: math.log(x, 10))

    sns.boxplot(x="tdp_value", y="time_total", hue="tdp_value", data=df)

    # turn off the legend
    plt.legend([],[], frameon=False)

    # plt.show()
    # save 
    plt.savefig("tdp-vs-time-boxplot.png")

