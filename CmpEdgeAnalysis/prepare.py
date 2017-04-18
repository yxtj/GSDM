import pickle
import numpy as np
import CmpEdgeGenerator as ceg
import analysis as anl
import display as dsp
import matplotlib.pyplot as plt
import importlib as imp

DATASETS = ['ahdh', 'abide', 'abide2', 'adni']
DATAPREFIX = '../data_abide/data-all'
METHODS = ['static', 'dynamic']

pathFC = DATAPREFIX + '/whole'
pathDFC1 = DATAPREFIX + '/p-s20-10/corr'
pathOut = DATAPREFIX + '/analysis'
typeCon = 2
typePat = 1


# data
def initData(prefix, pathCon, type, method):
    corr = ceg.loadConRef(pathCon, type, method)
    (m, s) = anl.getStatOfFC(corr)
    mse = anl.getMSEToNormal(corr, m, s)
    with open(prefix + '/analysis/ref-' + method + '.pickle', 'wb') as f:
        pickle.dump([corr, m, s, mse], f)


def reloadData(prefix, method):
    with open(prefix + '/analysis/ref-' + method + '.pickle', 'rb') as f:
        corr, m, s, mse = pickle.load(f)
    return (corr, m, s, mse)

# ------------
# static FC

# fcc = ceg.loadConRef(pathFC, typeCon, METHODS[0])
# fcp = ceg.loadConRef(pathFC, typePat, METHODS[0])
# with open(DATAPREFIX + '/analysis/fc.pickle', 'wb') as f:
#     pickle.dump([fcc, fcp], f)

def reloadFC():
    with open(DATAPREFIX + '/analysis/fc.pickle', 'rb') as f:
        fcc, fcp = pickle.load(f)
    return fcc, fcp

# corr1, s1, m1, mse1 = reloadData(DATAPREFIX, METHODS[0])
# corr2, s2, m2, mse2 = reloadData(DATAPREFIX, METHODS[1])

# ------------
# dynamic FC

# cLoader = ceg.cl.CorrLoader(pathDFC1)
# dfcc = cLoader.loadSnapshots(typeCon)
# with open(DATAPREFIX + '/analysis/dfc-c.pickle', 'wb') as f:
#     pickle.dump(dfcc, f)
# dfcp = cLoader.loadSnapshots(typePat)
# with open(DATAPREFIX + '/analysis/dfc-p.pickle', 'wb') as f:
#     pickle.dump(dfcp, f)

def reloadDFC():
    with open(DATAPREFIX + '/analysis/dfc-c.pickle', 'rb') as f:
        dfcc = pickle.load(f)
    with open(DATAPREFIX + '/analysis/dfc-p.pickle', 'rb') as f:
        dfcp = pickle.load(f)
    return dfcc, dfcp
