import os, sys
import DataLoader as dl
import GraphLoader as gl
from prepare import *


def writeEdgeList(fn, elist):
    el = elist if isinstance(elist, list) else list(elist)
    with open(fn, 'w') as f:
        f.write(str(len(el)))
        f.write('\n')
        for (i, j) in el:
            f.write('%d,%d' % (i, j))
            f.write('\n')


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
            temp = ' '.join(str(t) for t in g[i])
            f.write(temp)
            if len(temp) != 0:
                f.write(' ')
            f.write('\n')


def main(path: str, typePos: int, typeNeg: int, pruneTh: str, relativeTh: str, outEdge: bool):
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
    if outEdge:
        print('Outputing edge list (wk:', len(elistw), ', en:', len(eliste), ')...')
        writeEdgeList(path + '/edge-wk-' + suffix + '.txt', elistw)
        writeEdgeList(path + '/edge-en-' + suffix + '.txt', eliste)


    print('Generating pruned weakened edges...')
    l = dl.getFileNames(path + '/weakened/', typePos)
    gLoader = gl.GraphLoader(path + '/weakened/')
    if not os.path.exists(path + '/weakened-' + suffix):
        os.makedirs(path + '/weakened-' + suffix)
    cnt = 0
    for fn in l:
        g = gLoader.loadOne(fn)
        g2 = __removeEdgesUndir(g, elistw)
        writeGraph(path + '/weakened-' + suffix + '/' + fn, g2)
        cnt += 1
        if cnt % 200 == 0:
            print('  weakened processed', cnt)

    print('Generating pruned enhanced edges...')
    l = dl.getFileNames(path + '/enhanced/', typePos)
    gLoader = gl.GraphLoader(path + '/enhanced/')
    if not os.path.exists(path + '/enhanced-' + suffix):
        os.makedirs(path + '/enhanced-' + suffix)
    cnt = 0
    for fn in l:
        g = gLoader.loadOne(fn)
        g2 = __removeEdgesUndir(g, eliste)
        writeGraph(path + '/enhanced-' + suffix + '/' + fn, g2)
        cnt += 1
        if cnt % 200 == 0:
            print('  weakened processed', cnt)

    print('Finished.')


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Prune the compared edges with the FC correlation\n'
              'Usage: <path> <type-pos> <type-neg> [min-mean] [min-rel-diff] [out-edge]\n'
              '  <path>: the path to the edge folder (should contains subfolders "enhanced" and "weakened");\n'
              '  <type-pos>: type(s) for the divisor, separated with "," for multiple types;\n'
              '  <type-neg>: type(s) for the dividend, separated with "," for multiple types;\n'
              '  <min-mean>: (=0.3) the minimum value for the absolute mean correlation;\n'
              '  <min-rel-diff>: (=0.1) the minimum absolute value for the relative difference on mean value;\n'
              '  <out-edge>: (=False) weather to put a copy of the used edge list at path foler with name.')
        exit()
    # path = '../data_abide/data-all/dis-2/'
    path = sys.argv[1]
    typePos = sys.argv[2]
    typeNeg = sys.argv[3]
    pruneTh = sys.argv[4] if len(sys.argv) > 4 else '0.3'
    relativeTh = sys.argv[5] if len(sys.argv) > 5 else '0.1'
    outEdge = False
    if len(sys.argv) > 6:
        if sys.argv[6].lower() in {'1', 'true', 'yes'}:
            outEdge = True
        elif sys.argv[6].lower() in {'0', 'false', 'no'}:
            outEdge = False
        else:
            print('Warning: value of [out-edge] is illegal, assumed False.')
    main(path, typePos, typeNeg, pruneTh, relativeTh, outEdge)
