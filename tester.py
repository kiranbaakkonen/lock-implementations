import subprocess
import sys
import os
import numpy as np
import matplotlib.pyplot as plt


# xdata = rows of xdata to be plotted
# ydata = rows of y data to be plotted
# legend = array of size 3 of [title, xaxis, y axis]
def plotter(xdata, ydata, legend, titles, name):
    p, n = np.shape(xdata)
    n1, p1 =  np.shape(ydata)

    print(ydata)

    if(n != 1 or p != p1 or n1 < 1):
        print("plotter improperly used")
        return
    
    fig = plt.figure(figsize=(10,5))
    for i in range(0, n1):
        print(ydata[i])
        plt.plot(xdata, ydata[i,:])
    
    
    plt.title(titles[0])
    plt.xlabel(titles[1])
    plt.ylabel(titles[2])
    plt.legend(legend)

    if(os.path.exists(name)):
        os.remove(name)

    fig.savefig(name, bbox_inches='tight', dpi=250)
    



def correctness_test():
    print("Running correctness test")
    sum_one = 0
    sum_two = 0
    sum_three = 0
    sum_four = 0

    # one thread
    print("Running with 1 thread")
    for i in range(0, 10):
        sum_one += subprocess.call(["./counter", "-ct", "1", "10000000", "0"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
    
    if(sum_one == 10):
        print("pass: one thread")
    else:
        print(f"FAIL: one thread ({sum_one} passed out of 10)")

    # two threads
    print("Running with 2 threads")
    for i in range(0,10):
        sum_two += subprocess.call(["./counter", "-ct", "2", "10000000", "0"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)

    if(sum_two == 10):
        print("pass: one thread")
    else:
        print(f"FAIL: one thread ({sum_two} passed out of 10)")

    # four threads
    print("Running with 4 threads")
    for i in range(0,10):
        sum_three += subprocess.call(["./counter", "-ct", "4", "10000000", "0"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)

    if(sum_three == 10):
        print("pass: one thread")
    else:
        print(f"FAIL: one thread ({sum_three} passed out of 10)")

    # eight threads
    print("Running with 8 threads")
    for i in range(0,10):
        sum_four += subprocess.call(["./counter", "-ct", "8", "10000000", "0"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)

    if(sum_four == 10):
        print("pass: one thread")
    else:
        print(f"FAIL: one thread ({sum_four} passed out of 10)")

    print("correctness test completed")
    print(f"{sum_one + sum_two + sum_three + sum_four}/40 tests passed\n")



def idle_overhead():
    print("Running idle overhead test")

    results = [0,0,0,0,0]
    for i in range(0,20):
        subprocess.call(["./counter", "-wbct", "1", "10000000", "0"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
        with open("results.txt") as file:
            lines = file.readlines()
            for j in range(0,5):
                results[j] += float(lines[j])

    for k in range(0,5):
        results[k] = results[k]/20
    
    print(f"Serial avg = {results[0]}")
    print(f"TAS avg = {results[1]}")
    print(f"Backoff avg = {results[2]}")
    print(f"Mutex avg = {results[3]}")
    print(f"Array avg = {results[4]}")

    print(f"Serial throughput = {10000000/results[0]}")
    print(f"TAS throughput = {10000000/results[1]}")
    print(f"Backoff throughput = {10000000/results[2]}")
    print(f"Mutex throughput = {10000000/results[3]}")
    print(f"Array throughput = {10000000/results[4]}")

    
    print(f"TAS speedup = {results[0]/results[1]}")
    print(f"Backoff speedup = {results[0]/results[2]}")
    print(f"Mutex speedup = {results[0]/results[3]}")
    print(f"Array speedup = {results[0]/results[4]}")

    


def lock_scaling():

    print("Running lock scaling test")

    results = np.zeros((4,5), dtype='float')
    temp_arr = np.zeros((5,1))
    xdata = np.zeros((5,1))
    param = np.array([1,2,4,8,14])

    print("\tn\t\t\tserial\t\t\ttas\t\t\tbackoff\t\t\tmutex\t\t\tarray")

    for i in range(0,5):
        temp_arr = np.zeros((5,1))
        xdata[i][0] = param[i]
        for j in range(0,20):
                subprocess.call(["./counter", "-wbct", str(param[i]), "10000000", "0"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
                with open("results.txt") as file:
                    lines = file.readlines()
                    for k in range(0,5):
                        temp_arr[k] += float(lines[k])
        for l in range(0,5):
            temp_arr[l] = temp_arr[l]/20
        
        for a in range(0,4):
            results[a][i] = temp_arr[0]/temp_arr[a+1]

        print(f"\t{param[i]}\t{temp_arr[0]}\t{temp_arr[1]}\t{temp_arr[2]}\t{temp_arr[3]}\t{temp_arr[4]}")

    plotter(xdata, results, ["TAS", "Backoff", "pthread_mutex", "Array"], ["Lock scaling", "number of thread", "Parallel speedup (serial throughput/parallel throughput)"], "lockscaling.png")
        


def fairness():

    print("Running fairness test")

    results = np.zeros((4,4), dtype='float')
    temp_arr = np.zeros((4,1))
    param = [2,4,8,14]
    xdata = np.zeros((4,1))

    print("\tn\t\t\ttas\t\t\tbackoff\t\t\tmutex\t\t\tarray")

    for i in range(0,4):
        temp_arr = np.zeros((4,1))
        xdata[i][0] = param[i]
        for j in range(0,20):
                subprocess.call(["./counter", "-ft", str(param[i]), "10000000", "0"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
                with open("results.txt") as file:
                    lines = file.readlines()
                    for k in range(0,4):
                        temp_arr[k] += float(lines[k])
        for l in range(0,4):
            temp_arr[l] = temp_arr[l]/20
            results[l][i] = temp_arr[l]
        print(f"\t{param[i]}\t{temp_arr[0]}\t{temp_arr[1]}\t{temp_arr[2]}\t{temp_arr[3]}")

    plotter(xdata, results, ["TAS", "Backoff", "pthread_mutex", "Array"], ["Fairness", "number of thread", "Deviation of increments per thread"], "fairness.png")

    

def print_usage():
    print("Tester usage as follows:")
    print("python3 tester.py [options] [test_number]")
    print("options:")
    print("-ct: run the correctness test\n")
    print("-pt: run the performance test\n\t 1 = idle overhead \n\t 2 = lock scaling \n\t 3 = fairness test")

def main():
    if(len(sys.argv) != 3 and sys.argv[1] == "-pt"):
        print_usage()
        return -1
    elif(len(sys.argv) != 2 and sys.argv[1] == "-ct"):
        print_usage()
        return -1
    

    if(sys.argv[1] == "-ct"):
        correctness_test()
        return 1
    elif(sys.argv[1] == "-pt"):
        if(sys.argv[2] == "1"):
            idle_overhead()
            return 1
        elif(sys.argv[2] == "2"):
            lock_scaling()
            return 1
        elif(sys.argv[2] == "3"):
            fairness()
            return 1 
        else:
            print_usage()
            return -1
    else:
        print_usage()
        return -1

main()
