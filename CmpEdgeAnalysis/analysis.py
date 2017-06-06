import numpy as np
from scipy.stats import norm
from typing import Union, List, Tuple


def getStatOfFC(fc: np.ndarray):
    assert fc.ndim == 3 and fc.shape[1] == fc.shape[2]
    m = np.mean(fc, 0)
    # s = np.std(corr, 0, ddof=1)
    s = np.std(fc, 0)
    return (m, s)


def getStatOfDFC(dfc: List[np.ndarray]):
    nSub = len(dfc)
    assert nSub > 0 and dfc[0].ndim == 3
    nNode = len(dfc[0][0])
    ma = np.zeros([nSub, nNode, nNode])
    sa = np.zeros([nSub, nNode, nNode])
    tm = np.zeros([nNode, nNode])
    ts = np.zeros([nNode, nNode])
    tn = 0
    for i in range(nSub):
        n = len(dfc[i])
        t1 = np.sum(dfc[i], 0)
        t2 = np.var(dfc[i], 0)
        ma[i] = t1 / n
        sa[i] = np.sqrt(t2)
        tm += t1
        ts += t2 * n
        tn += n
    mg = tm / tn
    sg = np.sqrt(ts / tn)
    return ma, sa, mg, sg


def getMSEToNormal_one(data, m, s):
    if m == None or s == None:
        m, s = norm.fit(data)
    if s == 0:
        return 0.0
    l = len(data)
    scale = 1./l
    d = sorted(data)
    tc = norm.cdf(d, m, s)
    err = sum((tc[k] - (k+1)*scale)**2 for k in range(l)) / l
    return err


def getMSEToNormal(corr, m, s):
    (l, size_i, size_j) = corr.shape
    err = np.zeros([size_i, size_j])
    scale = 1./l
    for i in range(size_i):
        for j in range(size_j):
            if s[i][j] == 0:
                continue
            d = sorted(corr[:,i,j])
            tc = norm.cdf(d, m[i][j], s[i][j])
            err[i][j] = sum((tc[k] - (k+1)*scale)**2 for k in range(l)) / l
    return err


def pickEdgeOfAll(corra, i,j):
    res = []
    for sub in corra:
        res.extend(sub[:,i,j])
    return res

# used as cross subject derivation
def getGlobalMSE(corra, m, s, i : int, j : int):
    nSub = len(corra)
    res = np.zeros(nSub)
    for k in range(nSub):
        res[k] = getMSEToNormal_one(corra[k][:, i, j], m, s)
    return res


def setCond(mat: np.ndarray, cond: np.ndarray, val=0):
    assert mat.shape == mat.shape, \
        "the shape of the original matrix should be identical to the condition matrix's"
    m = mat.copy()
    m[cond] = val
    return m


def removeCond(mat: np.ndarray, cond: np.ndarray=None, val=0):
    l = mat.flatten()
    if cond is not None and cond.size == mat.size:
        l[cond.ravel()] = val
    l = np.delete(l, np.argwhere(l == val))
    return l


def mapIndexLinear2D(idx, n):
    return idx//n, idx%n


def mapIndexTriangleU2D(idx, n, k=0):
    x, y = 0, 0
    rest = idx
    for i in range(n - k, 0, -1):
        if rest < i:
            y = x + k + rest
            rest = 0
            break
        else:
            x += 1
            rest -= i
    return x, y


def mapIndexTriangleD2D(idx, n, k=0):
    x, y = k, 0
    rest = idx
    for i in range(1, n - k + 1):
        if rest < i:
            y = rest
            rest = 0
            break
        else:
            x += 1
            rest -= i
    return x, y


# given a sorted l, threshold of actual value(s), percentile requirement(s) (in 0~100). boundary included
# return index of the tail(s). value range: -1 ~ len(l). meaningful result: (0, res[0]] U [res[1], len(n))
# if dir='B': return 2 indexes of the first element satisfying the condition(s) for the two tails(<= and >=)
# if dir='L' or 'U': return the first index of the lower(<=)/upper(>=) tail which satisfying the condition(s)
def pickTailIndex(l, thres=None, percentile=None, dir:str='B'):
    if thres is None and percentile is None:
        return l
    dir = dir.upper()
    if dir not in {'B', 'U', 'L'}:
        raise ValueError('dir parameter should be one of {"B", "U", "L"}')

    n = len(l)
    if dir == 'B':
        if (thres is not None and len(thres) != 2) or (percentile is not None and len(percentile) != 2):
            raise ValueError('parameter should have two values for dir: "B"')
        rngt = [np.searchsorted(l, thres[0], side='right') - 1, np.searchsorted(l, thres[1])] if thres is not None else [n, 0]
        rngp = [int(percentile[0]*n/100) - 1, int(np.ceil(percentile[1]*n/100 - 1))] if percentile is not None else [n, 0]
        rng = (min(rngt[0], rngp[0]), max(rngt[1], rngp[1]))
        return rng
    else:
        if (thres is not None and len(thres) != 1) or (percentile is not None and len(percentile) != 1):
            raise ValueError('parameter should have one values for dir: "U" and "L"')
        if dir == 'U':
            rngt = np.searchsorted(l, thres, side='right') - 1 if thres is not None else 0
            rngp = int(percentile*n/100) - 1 if percentile is not None else 0
            rng = max(rngt, rngp)
        else:
            rngt = np.searchsorted(l, thres) if thres is not None else n
            rngp = int(np.ceil(percentile*n/100 - 1)) if percentile is not None else n
            rng = min(rngt, rngp)
        return rng


def pickTopEdges(mat: np.ndarray, thres=None, percentile=None, dir:str='B'):
    if thres is None and percentile is None or mat.ndim != 2:
        return
    dir = dir.upper()
    if dir not in {'B', 'U', 'L'}:
        raise ValueError('dir parameter should be one of {"B", "U", "L"}')

    nNode = mat.shape[0]
    # idx = np.triu_indices_from(mat)
    # l = mat[idx]
    l = mat.ravel()
    n = len(l)
    idx = np.argsort(l, None)
    ll = l[idx]
    rng = pickTailIndex(ll, thres, percentile, dir)
    res = ([], [])
    for i in range(0, rng[0] + 1):
        res[0].append(mapIndexLinear2D(idx[i], nNode))
    for i in range(n - 1, rng[1], -1):
        res[1].append(mapIndexLinear2D(idx[i], nNode))
    return res


def getCoChangeDirect(dynamics: Union[list, np.ndarray], refs: Union[list, np.ndarray],
                shifts: Union[list, np.ndarray]=None) -> np.ndarray:
    assert len(dynamics) == len(refs) and (shifts is None or len(shifts) == len(refs)), 'parameters do not match'
    assert len(dynamics) >= 2 and len(dynamics[0]) != 0 and np.isscalar(refs[0])
    n = len(dynamics)
    m = len(dynamics[0])
    cless = np.array(np.zeros(m) + 1, np.bool)
    cgreat = np.array(np.zeros(m) + 1, np.bool)
    for i in range(n):
        cless = np.logical_and(cless, dynamics[i] < refs[i])
        cgreat = np.logical_and(cgreat, dynamics[i] > refs[i])
    co = np.zeros(m, int)
    co[cless] = -1
    co[cgreat] = 1
    return co


def getCoChange(dfc: np.ndarray, ref: np.ndarray, indexes: Union[list, np.ndarray],
                shift: np.ndarray=None) -> np.ndarray:
    assert dfc.ndim == 3 and dfc[0].shape == ref.shape and (shift is None or shift.shape == ref.shape)
    assert len(indexes) >= 2 and isinstance(indexes[0], tuple or list or np.ndarray)
    n, m = dfc.shape[0:1]
    dm = []
    rf = []
    for i, j in indexes:
        dm.append(dfc[:, i, j])
        rf.append(ref[i, j])
    return getCoChangeDirect(dm, rf, shift)


def getCoTrendDirect(dynamics: Union[list, np.ndarray], shifts: Union[list, np.ndarray] = None) -> np.ndarray:
    assert shifts is None or len(shifts) == len(dynamics), 'parameters do not match'
    assert len(dynamics) >= 2 and len(dynamics[0]) != 0
    n = len(dynamics)
    m = len(dynamics[0])
    cdown = np.array(np.zeros(m - 1) + 1, np.bool)
    cup = np.array(np.zeros(m - 1) + 1, np.bool)
    for i in range(n):
        ref = dynamics[i][1:] - dynamics[i][:-1]
        cdown = np.logical_and(cdown, ref < 0)
        cup = np.logical_and(cup, ref > 0)
    co = np.zeros(m - 1, int)
    co[cdown] = -1
    co[cup] = 1
    return co


def getCoTrend(dfc: np.ndarray, indexes: Union[list, np.ndarray], shift: np.ndarray=None) -> np.ndarray:
    assert dfc.ndim == 3 and (shift is None or shift.shape == dfc[0].shape)
    assert len(indexes) >= 2 and isinstance(indexes[0], tuple or list or np.ndarray)
    n, m = dfc.shape[0:1]
    dm = []
    for i, j in indexes:
        dm.append(dfc[:, i, j])
    return getCoTrendDirect(dm, shift)

