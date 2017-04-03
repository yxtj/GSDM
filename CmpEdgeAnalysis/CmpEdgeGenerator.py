import sys, os, re
import CorrLoader as cl
import DataLoader as dl
import numpy as np
import matplotlib.pyplot as plt
import scipy.stats

# plot some single edge sample
def drawCorrDistri(dis, i, j, n = 40):
    plt.hist(dis, n, [-1, 1], weights=np.zeros_like(dis) + 1. / len(dis))
    plt.xlabel('Correlation')
    plt.ylabel('Frequency')
    plt.xlim([-1, 1])
    plt.title('Correlation Frequency of Edge ' + str(i) + '-' + str(j))
    plt.grid(True)
    plt.show()


def loadConRef(pathCon, typeCon, refMethod):
    cLoader = cl.CorrLoader(pathCon)
    if refMethod == 'static':
        subs = cLoader.loadWhole(typeCon)
    elif refMethod == 'dynamic':
        subs = cLoader.loadDynamic(typeCon)
    else:
        print('Unsupported.')
        exit(0)

    nSub = len(subs)
    nNode = len(subs[0])

    # ref = [[0.0 for i in range(nSub)] for i in range(nNode)] for i in range(nNode)]
    ref = np.zeros([nNode, nNode, nSub])
    for k in range(nSub):
        sub = subs[k]
        ref[:, :, k] = sub
    return ref


def getStatOfRef(corr):
    m = np.mean(corr, 2)
    s = np.std(corr, 2)
    return (m, s)


def getMLEToNormal_one(data, m, s):
    if s == 0:
        return 0.0
    l = len(data)
    scale = 1./l
    d = sorted(data)
    tc = scipy.stats.norm.cdf(d, m, s)
    err = sum((tc[k] - (k+1)*scale)**2 for k in range(l))
    return err


def getMLEToNormal(corr, m, s):
    (size_i, size_j, l) = corr.shape
    err = np.zeros([size_i, size_j])
    scale = 1./l
    for i in range(size_i):
        for j in range(size_j):
            if i == j:
                continue
            d = sorted(corr[i][j])
            tc = scipy.stats.norm.cdf(d, m[i][j], s[i][j])
            err[i][j] = sum((tc[k] - (k+1)*scale)**2 for k in range(l))
    return err


def drawRefCorrInfo(m, s, mle):
    fig = plt.figure()

    plt.subplot(2, 2, 1)
    plt.imshow(m, 'RdBu', clim=(-1, 1))
    plt.colorbar()
    plt.title('mean')

    plt.subplot(2, 2, 2)
    plt.hist(m.reshape(m.size), 200, [-1, 1], weights=np.zeros(m.size) + 1./m.size)
    plt.grid(True)
    plt.title('distr. of mean')

    plt.subplot(2, 2, 3)
    plt.imshow(s, 'Blues')

    plt.colorbar()
    plt.title('std. var.')

    plt.subplot(2, 2, 4)
    plt.imshow(mle, 'Blues')
    plt.colorbar()
    plt.title('MLE to norm. dis.')

    plt.tight_layout()
    #plt.show()


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
    (m, s) = getStatOfRef(refCorr)
    mle = getMLEToNormal(refCorr, m, s)
    drawRefCorrInfo(m, s, mle)
    plt.savefig(pathOut+'/figure/ref-'+refMethod+'.eps')
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


if __name__ == '__main__':
    if len(sys.argv) < 6 or len(sys.argv) > 8:
        print('Generate compare-based graph for each snapshot.\n'
              'Usage: <pathCon> <pathPat> <pathOut> <typeCon> <typePat> [factor] [refMethod]\n'
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
        exit(0)
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
            exit(0)
    main(pathCon, pathPat, pathOut, typeCon, typePat, factor, refMethod)
