import sys, os, re
import CorrLoader as cl
import DataLoader as dl
import numpy as np
import matplotlib.pyplot as plt
import analysis as anl


def loadConRef(pathCon, typeCon, loadMethod):
    cLoader = cl.CorrLoader(pathCon)
    subs = None
    if loadMethod == 'static':
        subs = cLoader.loadWhole(typeCon)
    elif loadMethod == 'dynamic':
        subs = cLoader.loadDynamic(typeCon)
    else:
        print('Unsupported.')
        exit(0)
    return subs


class RefMethod:
    def __init__(self, method):
        regNum = r'(?:[0-9]*[.])?[0-9]+'
        pat = '(fix):(' + regNum + ')|(exp):(' + regNum + '),(' + regNum + ')'
        m = re.match(pat, method)
        assert m and m.group() == method, 'Unsupported refMethod: ' + method
        if m.group(1):
            self.name = 'fix'
            self.parm = float(m.group(2))
        elif m.group(3):
            self.name = 'exp'
            self.parm = (float(m.group(4)), float(m.group(5)))
        else:
            print('error in parsing refMethod')

    def cal_fix(self, corr, m, s):
        c = corr - m
        fs = self.parm * s
        res = np.zeros_like(corr, np.int8)
        res[c > fs] = 1
        res[c < -fs] = -1
        res[np.diag_indices(len(res))] = 0
        return res

    def cal_exp(self, corr, m, s):
        c = corr - m
        fs = np.exp(-(m / self.parm[1]) ** 2) * self.parm[0] / self.parm[1] * s
        res = np.zeros_like(corr, np.int8)
        res[c > fs] = 1
        res[c < -fs] = -1
        res[np.diag_indices(len(res))] = 0
        return res

    def cal(self, corr, m, s):
        if self.name == 'fix':
            return self.cal_fix(corr, m, s)
        else:
            return self.cal_exp(corr, m, s)


def generateCmpGraph(corr, m, s, method: RefMethod):
    return method.cal(corr, m, s)


def writeCmpGraph(fn, mat, val):
    n = len(mat)
    with open(fn, 'w') as fout:
        fout.write(str(n))
        fout.write('\n')
        for i in range(n):
            fout.write(str(i))
            fout.write('\t')
            x = np.argwhere(mat[i] == val)
            fout.write(' '.join(str(t[0]) for t in x))
            fout.write(' \n')


def writeCmpGraphMatrix(fn, g):
    MAGIC_BYTE = 0x10
    with open(fn, 'wb') as fout:
        fout.write(bytes((MAGIC_BYTE,)))
        # TODO


def main(pathCon, pathPat, pathOut, typeCon, typePat, refMethod, loadMethod):
    # load reference data
    print('Loading reference...')
    refCorr = loadConRef(pathCon, typeCon, loadMethod)
    (m, s) = anl.getStatOfFC(refCorr)
    # mse = anl.getMSEToNormal(refCorr, m, s)
    # dsp.drawRefCorrInfo(m, s, mse)
    # plt.savefig(pathOut+'/analysis/ref-'+refMethod+'.eps')

    # load patient's dynamic snapshots
    print('Preparing files for output...')
    l = dl.getFileNames(pathPat, typePat)
    if not os.path.exists(pathOut + '/enhanced/'):
        os.makedirs(pathOut + '/enhanced/')
    if not os.path.exists(pathOut + '/weakened/'):
        os.makedirs(pathOut + '/weakened/')
    cLoader = cl.CorrLoader(pathPat)
    print('Generating output graphs...')
    for i in range(len(l)):
        if i != 0 and i % 200 == 0:
            print('  Processed:', i, '/', len(l))
        fn = l[i]
        c = cLoader.loadOne(fn)
        c = generateCmpGraph(c, m, s, refMethod)
        writeCmpGraph(pathOut + '/enhanced/' + fn, c, 1)
        writeCmpGraph(pathOut + '/weakened/' + fn, c, -1)
    print('Finished:', len(l))

__USAGE__ = 'Generate compare-based graph for each snapshot.\n' \
            'Usage: <pathCon> <pathPat> <pathOut> <typeCon> <typePat> [refMethod] [loadMethod]'

if __name__ == '__main__':
    if len(sys.argv) == 1:
        print(__USAGE__ + '\nuse --help for detailed help')
        exit()
    elif '--help' in sys.argv or len(sys.argv) < 6 or len(sys.argv) > 8:
        print(__USAGE__ + '\n'
              '  <pathCon>: path to the healthy control group\'s correlation files (whole scan in one)\n'
              '  <pathPat>: path to the patient groups\'s correlation files (multiple snapshots)\n'
              '  <pathOut>: path to the output folder. Output files keeps the name in <pathPat>\n'
                          '  [refMethod]: (=fix:1) the factor used to define discriminative edge from the reference, '
                          'Condition for e: e < m-<fac>*s OR m+<fac>*s > e, where m is the ref. mean and s is ref. std. '
                          'Support:\n'
                          '    fix:<val> : give a fixed positive factor, <fac>=<val>.\n'
                          '    exp:<norm>,<sigma> : <fac> is larger when m is small: <fac>=exp(-|m/<sigma>|**2)*<norm>/<sigma>. '
                          'Quick guide: <sigma> is <fac> when m = 1 or -1, and <norm>/<sigma> is <fac> when m=0:\n'
                          '  [loadMethod]: (=dynamic) the method of merging reference data. Support:\n'
              '    static: merge everything equally. Fit case: one correlation using whole scan period\n'
                          '    dynamic: snapshots of the same subject are equally, subjects are equally. '
                          'Fit case: multiple snapshots on each subject\n'
              # '    periodic: generate a periodic reference. Not supported yet\n'
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
    refMethod = RefMethod(sys.argv[6] if len(sys.argv) > 6 else "fix:1")
    loadMethod = 'dynamic'
    if len(sys.argv) > 7:
        loadMethod = sys.argv[7]
        assert loadMethod in {'static', 'dynamic'}, 'Unsupported loadMethod: ' + loadMethod
    main(pathCon, pathPat, pathOut, typeCon, typePat, refMethod, loadMethod)
