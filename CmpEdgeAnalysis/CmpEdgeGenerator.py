import sys, os, re
import CorrLoader as cl
import DataLoader as dl
import numpy as np
import matplotlib.pyplot as plt
from display import *
import analysis as anl


def loadConRef(pathCon, typeCon, refMethod):
    cLoader = cl.CorrLoader(pathCon)
    subs = None
    if refMethod == 'static':
        subs = cLoader.loadWhole(typeCon)
    elif refMethod == 'dynamic':
        subs = cLoader.loadDynamic(typeCon)
    else:
        print('Unsupported.')
        exit(0)
    return subs


def generateCmpGraph(corr, factor, m, s):
    # only support static & dyanmic method
    c = corr - m
    fs = factor * s
    res = np.array(c > fs, np.int8) - np.array(c < -fs, np.int8)
    return res


def writeCmpGraph(fn, g):
    fout = open(fn, 'w')
    MAGIC_BYTE = 0x10
    fout.write(len(g))
    fout.write('\n')
    for line in g:
        for v in line:
            fout.write(str(v))
            fout.write(' ')
        fout.write('\n')
    fout.close()


def main(pathCon, pathPat, pathOut, typeCon, typePat, factor, refMethod):
    # load reference data
    refCorr = loadConRef(pathCon, typeCon, refMethod)
    (m, s) = anl.getStatOfRef(refCorr)
    mse = anl.getMSEToNormal(refCorr, m, s)
    drawRefCorrInfo(m, s, mse)
    plt.savefig(pathOut+'/analysis/ref-'+refMethod+'.eps')
    # plot
    # i=1;j=2;
    # drawCorrDistri(refCorr[i][j], i, j)

    # load patient's dynamic snapshots
    l = dl.getFileNames(pathPat, typePat)
    cLoader = cl.CorrLoader(pathPat)
    for fn in l:
        c = cLoader.loadOne(fn)
        c = generateCmpGraph(c, factor, m, s)
        writeCmpGraph(pathOut + '/' + fn, c)

__USAGE__ = 'Generate compare-based graph for each snapshot.\n' \
            'Usage: <pathCon> <pathPat> <pathOut> <typeCon> <typePat> [factor] [refMethod]'

if __name__ == '__main__':
    if len(sys.argv) == 1:
        print(__USAGE__ + '\nuse --help for detailed help')
        exit()
    elif '--help' in sys.argv or len(sys.argv) < 6 or len(sys.argv) > 8:
        print(__USAGE__ + '\n'
              '  <pathCon>: path to the healthy control group\'s correlation files (whole scan in one)\n'
              '  <pathPat>: path to the patient groups\'s correlation files (multiple snapshots)\n'
              '  <pathOut>: path to the output folder. Output files keeps the name in <pathPat>\n'
              '  [factor]: (=3) the significance factor for comparison.'
              '  [refMethod]: (=static) the metohd of merging reference data, support:\n'
              '    static: merge everything equally. Fit case: one correlation using whole scan period\n'
              '    dynamic: snapshots of the same subject are equally, subjects are equally.'
              ' Fit case: multiple snapshots on each subject\n'
              '    periodic: generate a periodic reference. Not supported yet\n'
              )
        exit()
    # pathCon = '../data_abide/data-all/whole'
    # pathPat = '../data_abide/data/p-s20-10/corr'
    # pathOut = '../data_abide/data/cmp/p-s20-10'
    # typeCon = 2
    # typePat = 1
    pathCon = sys.argv[1]
    pathPat = sys.argv[2]
    pathOut = sys.argv[3]
    typeCon = sys.argv[4]
    typePat = sys.argv[5]
    factor = 3.0
    if len(sys.argv) >= 7:
        factor = float(sys.argv[6])
    refMethod = 'static'
    if len(sys.argv) >= 8:
        refMethod = sys.argv[7]
        if refMethod not in {'static', 'dynamic', 'periodic'}:
            print('Unsupported refMethod: ' + refMethod)
            exit()
    main(pathCon, pathPat, pathOut, typeCon, typePat, factor, refMethod)
