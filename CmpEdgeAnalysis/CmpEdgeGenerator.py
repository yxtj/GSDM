import os,re
import CorrLoader as cl
import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt

typeCon=2
typePat=1

corrPathCon='../data_abide/data-all/whole'
corrPathPat='../data_abide/data/p-s20-10/corr'

cLoader=cl.CorrLoader(corrPathCon)
subs=cLoader.load(typeCon)

nSub=len(subs)
nNode=len(subs[0])

corr=[[[0.0 for i in range(nSub)] for i in range(nNode)] for i in range(nNode)]

for k in range(nSub):
    sub=subs[k]
    for i in range(nNode):
        for j in range(nNode):
            corr[i][j][k]=sub[i][j]

def drawCorrDistri(dis, i, j):
    plt.hist(dis, 2 * 10, [-1, 1], weights=np.zeros_like(dis) + 1. / len(dis))
    plt.xlabel('Correlation')
    plt.ylabel('Frequency')
    plt.xlim([-1, 1])
    plt.title('Correlation Frequency of Edge ' + str(i) + '-' + str(j))
    plt.grid(True)
    plt.show()

# plot
i=1;j=2;
drawCorrDistri(corr[i][j], i, j)
