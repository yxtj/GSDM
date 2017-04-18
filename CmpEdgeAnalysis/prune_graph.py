import os, sys
import DataLoader as dl
import GraphLoader as gl
from prepare import *


def sortUpEdgeList(elist):
    res = list(elist)
    for e in res:
        if e[0] > e[1]:
            e = e[1], e[0]
    res.sort()
    # unique
    f1, f2, l = 0, 1, len(elist)
    while f2 < l:
        if res[f1] != res[f2]:
            f1 += 1
            res[f1] = res[f2]
        f2 += 1
    if f1 != l - 1:
        res = res[:f1 + 1]
    return res


def __removeEdgesUndir(g, elist):
    el = sortUpEdgeList(elist)
    n = len(g)
    res = [[] for i in range(n)]
    for i in range(n):
        for x in g[i]:
            if x > i and ((i, x) in el or (x, i) in el):
                res[i].append(x)
                res[x].append(i)
    for i in range(n):
        res[i] = list(np.unique(res[i]))
    return res


# def removeEdges(g, elist, dir=False):
#     if dir:
#         return __removeEdgesDir(g, elist)
#     else:
#         return __removeEdgesUndir(g, elist)


def writeGraph(fn, g):
    n = len(g)
    with open(fn, 'w') as f:
        f.write(str(n))
        f.write('\n')
        for i in range(n):
            f.write(str(i))
            f.write('\t')
            f.write(' '.join(str(t) for t in g[i]))
            f.write(' \n')


def main(path: str, typePos: int, typeNeg: int, pruneTh: str, relativeTh: str):
    suffix = pruneTh + '-' + relativeTh
    print('Suffix:', suffix)

    print('Loading reference and mean difference...')
    (fcc, fcp) = reloadFC()
    mc = np.mean(fcc, 0)
    mp = np.mean(fcp, 0)
    dm = (mp - mc) / mc

    pth = float(pruneTh)
    rth = float(relativeTh)
    elistw, eliste = anl.pickTopEdges(anl.setCond(dm, np.abs(mc) <= pth), [-rth, rth])

    print('Generating pruned weakened edges...')
    l = dl.getFileNames(path + '/weakened/', typePos)
    gLoader = gl.GraphLoader(path + '/weakened/')
    if not os.path.exists(path + '/weakened-' + suffix):
        os.makedirs(path + '/weakened-' + suffix)
    for fn in l:
        g = gLoader.loadOne(fn)
        g2 = __removeEdgesUndir(g, elistw)
        writeGraph(path + '/weakened-' + suffix + '/' + fn, g2)

    print('Generating pruned enhanced edges...')
    l = dl.getFileNames(path + '/enhanced/', typePos)
    gLoader = gl.GraphLoader(path + '/enhanced/')
    if not os.path.exists(path + '/enhanced-' + suffix):
        os.makedirs(path + '/enhanced-' + suffix)
    for fn in l:
        g = gLoader.loadOne(fn)
        g2 = __removeEdgesUndir(g, eliste)
        writeGraph(path + '/enhanced-' + suffix + '/' + fn, g2)

    print('Finished.')


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Prune the compared edges with the FC correlation'
              'Usage: <path> <type-pos> <type-neg> <min-mean> <min-rel-diff>'
              '  <path>: the path to the edge folder;'
              '  <type-pos>: type(s) for the divisor, separated with "," for multiple types;'
              '  <type-neg>: type(s) for the dividend, separated with "," for multiple types;'
              '  <min-mean>: (=0.3) the minimum value for the absolute mean correlation;'
              '  <min-rel-diff>: (=0.1) the minimum absolute value for the relative difference on mean value.')
    # path = '../data_abide/data-all/dis-2/'
    path = sys.argv[1]
    typePos = sys.argv[2]
    typeNeg = sys.argv[3]
    pruneTh = sys.argv[4] if len(sys.argv) > 4 else '0.3'
    relativeTh = sys.argv[5] if len(sys.argv) > 5 else '0.1'
    main(path, typePos, typeNeg, pruneTh, relativeTh)
