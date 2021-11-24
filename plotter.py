import matplotlib.pyplot as plt
import numpy as np

overhead = np.array([
    [1.082, 1.043, 1.023],
    [1.098, 1.048, 1.019],
    [1.098, 1.056, 1.023]
])

dispatcher = np.array([1859, 1651, 2422, 2944,2580, 4484])

constant = np.array([
    [0.9393, 1.84588, 3.722968, 6.76878, 6.101637, 5.160014],
    [0.961411, 1.90954, 3.80753, 7.15983569, 9.612296, 6.61413],
    [0.9827447, 1.956259, 3.89841, 7.302835, 11.2676033, 8.93087],
    [0.98809, 1.97073, 3.950585, 7.729213, 12.194769, 10.182904]
])

uniform = np.array([
    [0.94148, 1.862163, 3.6999, 6.4481, 7.3885, 5.3295],
    [0.96648, 1.90688, 3.83053, 7.291, 10.657, 6.764],
    [0.989, 1.951, 3.901, 7.473, 11.07, 8.814],
    [0.9894, 1.9702, 3.9306, 7.639, 11.876, 10.182]
])

exponential = np.array([
    [0.952, 1.475, 2.568, 4.076, 5.773, 4.967],
    [0.973, 1.499, 2.586, 4.155, 5.942, 6.096],
    [0.982, 1.513, 2.595, 4.319, 6.302, 6.385],
    [0.99, 1.524, 2.603, 4.367, 6.609, 6.625]
])


nthreads = np.array([2,3,5,9,14,28])

w = np.array([200,400,800])

def plot_overhead():
    fig = plt.figure(figsize=(10,5))
    plt.plot(w, overhead[0], w, overhead[1], w, overhead[2])
    plt.xlabel("Work per packet")
    plt.ylabel("Overhead (serial queue time/serial time")
    plt.title("Parallel Overhead")
    plt.legend(["1 source", "8 sources", "13 sources"])
    fig.savefig("overhead.jpg", bbox_inches='tight', dpi=250)

def plot_dispatcher():
    fig = plt.figure(figsize=(10,5))
    plt.plot(nthreads, dispatcher)
    plt.xlabel("Number of threads")
    plt.ylabel("Dispatch rate (T*(n-1))/time")
    plt.title("Dispatch rate")
    fig.savefig("dispatcher.jpg", bbox_inches='tight', dpi=250)

def plot_constant():

    #plot constant speedup
    fig = plt.figure(figsize=(10,5))
    plt.plot(nthreads, constant[0], nthreads, constant[1], nthreads, constant[2], nthreads, constant[3])
    plt.xlabel("Number of threads")
    plt.ylabel("Speedup (serial time/parallel time")
    plt.title("Constant Speedup Test")
    plt.legend(["W = 1000", "W=2000", "w=4000", "W=8000"])
    fig.savefig("constant_speedup.jpg", bbox_inches='tight', dpi=250)

def plot_uniform():

    #plot constant speedup
    fig = plt.figure(figsize=(10,5))
    plt.plot(nthreads, uniform[0], nthreads, uniform[1], nthreads, uniform[2], nthreads, uniform[3])
    plt.xlabel("Number of threads")
    plt.ylabel("Speedup (serial time/parallel time")
    plt.title("Uniform Speedup Test")
    plt.legend(["W = 1000", "W=2000", "w=4000", "W=8000"])
    fig.savefig("uniform_speedup.jpg", bbox_inches='tight', dpi=250)

def plot_exponential():

    #plot constant speedup
    fig = plt.figure(figsize=(10,5))
    plt.plot(nthreads, exponential[0], nthreads, exponential[1], nthreads, exponential[2], nthreads, exponential[3])
    plt.xlabel("Number of threads")
    plt.ylabel("Speedup (serial time/parallel time")
    plt.title("Exponential Speedup Test")
    plt.legend(["W = 1000", "W=2000", "w=4000", "W=8000"])
    fig.savefig("exponential_speedup.jpg", bbox_inches='tight', dpi=250)

plot_overhead()
plot_dispatcher()
plot_constant()
plot_uniform()
plot_exponential()
