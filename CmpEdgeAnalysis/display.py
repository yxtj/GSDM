import numpy as np
import matplotlib.pyplot as plt
from  scipy.stats import norm


# plot some single edge sample
def drawCorrDistri(dis, i, j, n = 40):
    plt.hist(dis, n, [-1, 1], True)
    plt.xlabel('Correlation')
    plt.ylabel('Frequency')
    plt.xlim([-1, 1])
    plt.title('Distribution of the Correlation on Edge ' + str(i) + '-' + str(j))
    plt.grid(True)
    plt.show()

# plot 4 figures about the distribution of a collection
def drawRefCorrInfo(m, s, mse, bins=50):
    fig = plt.figure()

    plt.subplot(2, 2, 1)
    plt.imshow(m, 'RdBu', clim=(-1, 1))
    plt.colorbar()
    plt.title('mean over sub.')

    plt.subplot(2, 2, 2)
    plt.hist(m.reshape(m.size), 2*bins, [-1, 1], True)
    plt.grid(True)
    plt.title('distr. (over edge) of mean')

    plt.subplot(2, 2, 3)
    temp = np.abs(s / m)
    temp = temp.reshape(temp.size)
    plt.hist(temp, bins, [0, np.percentile(temp, 95)], True)
    plt.grid(True)
    plt.title('distr. of relative std. (first 95%)')

    plt.subplot(2, 2, 4)
    #plt.imshow(np.abs(mse/m), 'Blues')
    #plt.colorbar()
    temp = mse
    temp = temp.reshape(temp.size)
    plt.hist(temp, bins, [0, np.percentile(temp, 95)], True)
    plt.grid(True)
    plt.title('distr. of MSE to norm. (first 95%)')

    plt.tight_layout()
    #plt.show()


def showMeanDifference(mp, mc, th: float=None, xlabel: str=None, ylabel: str=None, title: str=None):
    dm2 = (mp - mc)/mc
    if th is not None and th > 0:
        dm2[np.abs(mc) < th] = 0
        dm2 = dm2.reshape(dm2.size)
        dm2 = np.delete(dm2, np.argwhere(dm2 == 0))
    else:
        dm2 = dm2.reshape(dm2.size)
    rng = np.percentile(dm2, [1, 99]) # use 1-percentile, 99-percentile
    bound = np.max(np.abs(rng))
    plt.hist(dm2, 100, [-bound, bound]);
    plt.grid(True)
    if xlabel is not None:
        plt.xlabel(xlabel)
    if ylabel is not None:
        plt.ylabel(ylabel)
    if title is not None:
        plt.title(title)


def showStdDifference(sp, sc, th: float=None, xlabel: str=None, ylabel: str=None, title: str=None):
    nNode = sp.shape[0]
    ds = (sp - sc)/sc
    for i in range(nNode):
        ds[i, i] = 0
    dsl = ds.ravel()
    if th is not None and th > 0:
        scr = sc.ravel()
        dsl = np.delete(dsl, np.argwhere(np.logical_or(scr == 0, scr > th)))
    rng = np.percentile(dsl, [1, 99]) # use 1-percentile, 99-percentile
    bound = np.max(np.abs(rng))
    plt.hist(dsl, 100, [-bound, bound]);
    plt.grid(True)
    if xlabel is not None:
        plt.xlabel(xlabel)
    if ylabel is not None:
        plt.ylabel(ylabel)
    if title is not None:
        plt.title(title)


def drawDynamicGrid(corra, r, i, j, legend=True, grid=True, sameLength=False):
    n = len(r)
    size = int(np.ceil(np.sqrt(n)))
    mg = sum(np.mean(snap[:,i,j]) for snap in corra) / len(corra)
    if sameLength:
        xmax = max(len(corra[k]) for k in r)
    for k in range(n):
        plt.subplot(size, size, k+1)
        l = range(len(corra[r[k]]))
        ml = np.mean(corra[r[k]][:,i,j])
        plt.plot(l, corra[r[k]][:,i,j], l, np.zeros_like(l)+ml, '--', l, np.zeros_like(l)+mg, '-.')
        plt.ylim([-1, 1])
        if not sameLength:
            xmax = len(corra[r[k]])
        plt.xlim([0, xmax])
        if legend:
            plt.title('sub-'+str(r[k]))
        if grid:
            plt.grid(True)
    plt.tight_layout()
    #plt.show()


def drawDynamic(dynamics: list, refs: list=None, showMean=None):
    if refs is not None:
        assert len(refs) == len(dynamics), 'each ref should have a corresponding dynamic'
    l = len(dynamics[0])
    i = 0
    for dyn in dynamics:
        plt.plot(dyn, 'C'+str(i))
        i += 1
    plt.xlim([0, l])
    plt.ylim([-1, 1])
    if refs is not None:
        i = 0
        for r in refs:
            plt.hlines(r, 0, l, 'C'+str(i), 'dashed')
            i += 1


def drawDistriNorm(data, bins):
    ax = plt.hist(data, bins, [-1,1], True)
    m = np.mean(data)
    s = np.std(data)
    l = np.linspace(-1,1,bins+1)
    plt.plot(l, norm.pdf(l, m, s))


def drawDistriNormCP(datac, datap, bins=50, showLegend = False):
    plt.hist([datac, datap], bins, [-1, 1], True, color=['C0','C1'])
    mc = np.mean(datac)
    sc = np.std(datac)
    mp = np.mean(datap)
    sp = np.std(datap)
    l = np.linspace(-1, 1, 2*bins + 1)
    plt.plot(l, norm.pdf(l, mc, sc), 'C0', l, norm.pdf(l, mp, sp), 'C1')
    if showLegend:
        plt.legend(['Control', 'Patient'], loc='upper left')
    plt.xlabel('Correlation')
    plt.ylabel('PDF')


def showCoDynamicGrid(dfc, mgc, r, indexes, showCoCmp=False, showCoTrend=False, nRow:int=None):
    nr = len(r)
    if nRow is None or nRow <= 0:
        nRow = int(np.ceil(np.sqrt(nr)))
    nCol = (nr + nRow - 1) // nRow
    for k in range(nr):
        plt.subplot(nRow, nCol, k+1)
        id = r[k]
        n = len(dfc[id])
        dynamics = []
        ref = []
        cl = np.array(np.zeros(n) + 1, np.bool)
        cg = np.array(np.zeros(n) + 1, np.bool)
        cd = np.array(np.zeros(n - 1) + 1, np.bool)
        cu = np.array(np.zeros(n - 1) + 1, np.bool)
        for idx in indexes:
            dynamics.append(dfc[id][:, idx[0], idx[1]])
            ref.append(mgc[idx[0], idx[1]])
            if showCoCmp:
                cl = np.logical_and(cl, dynamics[-1] < ref[-1])
                cg = np.logical_and(cg, dynamics[-1] > ref[-1])
            if showCoTrend:
                temp = dynamics[-1][1:] - dynamics[-1][:-1]
                cd = np.logical_and(cd, temp < 0)
                cu = np.logical_and(cu, temp > 0)
        drawDynamic(dynamics, ref)
        if showCoCmp:
            co = np.zeros(n)
            co[cl] = -1
            co[cg] = 1
            plt.plot(co, 'ok', markerfacecolor='none')
        if showCoTrend:
            co = np.zeros(n - 1)
            co[cd] = -1
            co[cu] = 1
            plt.plot(np.arange(1, n), co, 'xk')
    plt.tight_layout()

