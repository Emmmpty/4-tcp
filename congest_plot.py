__author__ = 'jmh081701'
from matplotlib import  pyplot as plt
import sys
import numpy as np
if __name__ == '__main__':
    if len(sys.argv)!=2:
        print('[usage]:python congest_plot.py congess_log.data')
        exit(0)
    congess_log = sys.argv[1]
    times    =[]
    cwnd     =[]
    ssthresh =[]
    sequence =[]
    cwnd_max =0
    time_max =0
    with open(congess_log) as fp:
        lines = fp.readlines()
        for line in lines:
            line =line.strip().split(",")
            times.append(float(line[0]))
            cwnd.append(float(line[1]))
            if float(line[1]) > cwnd_max:
               cwnd_max = float(line[1])
            ssthresh.append(float(line[2]))
            sequence.append(float(line[3])/1000)
    for i in range(len(times)-1,-1,-1):
        times[i]= times[i]-times[0]
        if times[i] > time_max :
               time_max = times[i]

    #print(times)
    plt.plot(times,cwnd,'r',label='cwnd')
    #plt.plot(times,ssthresh,'b',label='ssthressh')
    #plt.plot(times,sequence,'g*',label='sequence')
    plt.xlabel('time(/s)')
    plt.ylabel('size(/MSS)')
    plt.xticks([x for x in np.arange(0,time_max,0.3)])
    plt.yticks([y for y in np.arange(0,cwnd_max,0.3)])
    plt.legend()
    plt.gray()
    plt.show()
