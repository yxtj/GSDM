from prepare import *
# runfile('prepare.py')

(fcc, fcp) = reloadFC()
(dfcc, dfcp) = reloadDFC()

nNode = len(dfcc[0][0])
nSub = (len(dfcc), len(dfcp))
nSnap = ([len(sub) for sub in dfcc], [len(sub) for sub in dfcp])

mc, sc = np.mean(fcc, 0), np.std(fcc, 0)
mp, sp = np.mean(fcp, 0), np.std(fcp, 0)
mac, sac, mgc, sgc = anl.getStatOfDFC(dfcc)
map, sap, mgp, sgp = anl.getStatOfDFC(dfcp)


# analysis

# ------
# edge difference using all subjects' all snapshots
## correlation distribution
i=41;j=43
ec = anl.pickEdgeOfAll(dfcc, i, j)
ep = anl.pickEdgeOfAll(dfcp, i, j)
dsp.drawDistriNormCP(ec, ep, 100)

nRow = 3; nCol = 3
r = [(np.random.randint(nNode), np.random.randint(nNode)) for k in range(nRow*nCol)]
for k in range(nRow*nCol):
    plt.subplot(nRow, nCol, k+1)
    i, j = r[k]
    dsp.drawDistriNormCP(anl.pickEdgeOfAll(dfcc,i,j), anl.pickEdgeOfAll(dfcp,i,j), 25)
    #dsp.drawDistriNormCP(fcc[:, i, j], fcp[:, i, j], 25)
    plt.title('Edge: '+str(i)+'-'+str(j))
plt.tight_layout()
plt.show()
# findings: trivial edge->gaussian around 0, discriminative edge->not gaussian, peak near 1

## correlation variance distribution
plt.figure(figsize=[6.4, 2.4])
dsp.drawDistriNormCP(sc.ravel(), sp.ravel(), 300, True)
plt.xlabel('std.'); plt.xlim([0, 0.5])
plt.grid(True)
plt.tight_layout()
plt.show()

plt.figure(figsize=[6.4, 2.4])
dsp.drawDistriNormCP(sgc.ravel(), sgp.ravel(), 300, True)
plt.xlabel('std.'); plt.xlim([0.15, 0.45])
plt.grid(True)
plt.tight_layout()
plt.show()

## relationship between mean and std
mmin = np.floor(min(mc.min(), mp.min()) * 10) / 10  # floor to last 0.1
smax = np.ceil(max(sc.max(), sp.max()) * 20) / 20  # ceil to next 0.05

plt.subplot(2, 1, 1)
plt.plot(mc.ravel(), sc.ravel(), '.');
plt.xlabel('mean');
plt.ylabel('std.')
plt.subplot(2, 1, 2)
plt.plot(mp.ravel(), sp.ravel(), '.');
plt.xlabel('mean');
plt.ylabel('std.')
plt.tight_layout()
plt.show()

plt.subplot(2, 1, 1)
dsp.drawMeanStdRelation(mc, sc, [mmin, 1], [0, smax], clog=True, title='Control')
plt.subplot(2, 1, 2)
dsp.drawMeanStdRelation(mp, sp, [mmin, 1], [0, smax], clog=True, title='Patient')
plt.tight_layout()
plt.show()

# fit the mean-std relations curve
idx = np.triu_indices(nNode, 1)
x = mc[idx]
y = sc[idx]

fun1 = lambda t, alpha, sigma: np.exp(-(t / sigma) ** 2) * alpha  # 0-centralized
fun2 = lambda t, alpha, mu, sigma: np.exp(-((t - mu) / sigma) ** 2) * alpha
fun3 = lambda t, alpha, mu, sigma: np.exp(-((t - mu) / sigma) ** 2) * alpha / sigma

fun = fun3
import scipy.optimize

par, pcov = scipy.optimize.curve_fit(fun, x, y)

l = np.linspace(mmin, 1, 100)
plt.plot(x, y, '.')
plt.plot(l, fun(l, *par))
plt.show()

# ------
# relative difference about the mean value

dsp.showMeanDifference(mp, mc, None, 'relative mean difference to controls', '# of edges',
                       'distribution of mean difference over edges')
plt.show()


title = 'distr. of corr. difference over edges'
# vertical
plt.subplot(3, 1, 1)
dsp.showMeanDifference(mp, mc, None, None, '# of edges', title)
plt.subplot(3, 1, 2)
dsp.showMeanDifference(mp, mc, 0.1, None, '# of edges', title + ' (Org>0.1)')
plt.subplot(3, 1, 3)
dsp.showMeanDifference(mp, mc, 0.2, 'relative mean difference to controls', '# of edges', title + ' (Org>0.2)')
plt.tight_layout()
plt.show()

# horizontal
plt.figure(figsize=[9, 2])
plt.subplot(1, 3, 1)
dsp.showMeanDifference(mp, mc, None, None, '# of edges','No prune')
plt.subplot(1, 3, 2)
dsp.showMeanDifference(mp, mc, 0.1, 'relative mean difference', None, 'Org>0.1')
plt.subplot(1, 3, 3)
dsp.showMeanDifference(mp, mc, 0.2, None, None, 'Org>0.2')
plt.tight_layout()
plt.show()

# ------
# find the edges with large mean-value difference


def getNumChangedEdge(dml, th):
    dml.sort()
    idx = anl.pickTailIndex(dml, [-th, th])
    cnt = (max(0, idx[0] - 0), max(0, dml.size-idx[1]))
    return cnt

pruneTh = 0.1
dm = (mp - mc) / mc
dml = anl.removeCond(dm, np.abs(mc) <= pruneTh)

th = 0.2
cnt = getNumChangedEdge(dml, th)
print('# of edges with difference>='+str(th*100)+'%: weakened:',cnt[0],'enhanced',cnt[1])

def showNumChangedEdges(dml, thRng, ylog=False, legend=False, xlabel=True, ylabel=True):
    cntList = []
    for th in thRng:
        cntList.append(getNumChangedEdge(dml, th))
    data = np.array(cntList)
    plt.plot(thRng, data[:,0], '-o', thRng, data[:,1], '-x')
    if ylog:
        plt.yscale('log')
    if legend:
        plt.legend(['Weakened', 'Enhanced'])
    plt.grid(True)
    if xlabel:
        plt.xlabel('threshold')
    if ylabel:
        plt.ylabel('# of edges')

plt.figure(figsize=[9.6, 3.2])
plt.subplot(1, 3, 1)
showNumChangedEdges(anl.removeCond(dm, np.abs(mc) <= 0.1), np.arange(3, 16)*0.02, True, True)
plt.title('Org>0.1')
plt.subplot(1, 3, 2)
showNumChangedEdges(anl.removeCond(dm, np.abs(mc) <= 0.2), np.arange(3, 16)*0.02, True, True)
plt.title('Org>0.2')
plt.subplot(1, 3, 3)
showNumChangedEdges(anl.removeCond(dm, np.abs(mc) <= 0.3), np.arange(3, 16)*0.02, True, True)
plt.title('Org>0.3')
plt.tight_layout()
plt.show()

pruneTh = 0.3
th = 0.2
elistw, eliste = anl.pickTopEdges(anl.setCond(dm, np.abs(mc) <= pruneTh), [-th, th])

# ------
# relative difference about the standard derivation

dsp.showStdDifference(sp, sc, None, 'relative std. difference', '# of edges', 'No prune')
plt.show()

# horizontal
# FC
plt.figure(figsize=[9,2])
plt.subplot(1, 3, 1)
dsp.showStdDifference(sp, sc, None, None, '# of edges','No prune')
plt.subplot(1, 3, 2)
dsp.showStdDifference(sp, sc, 0.21, 'relative std. difference', None, 'Org<0.21')
plt.subplot(1, 3, 3)
dsp.showStdDifference(sp, sc, 0.18, None, None, 'Org<0.18')
plt.tight_layout()
plt.show()

# DFC
plt.figure(figsize=[9,2])
plt.subplot(1, 3, 1)
dsp.showStdDifference(sgp, sgc, None, None, '# of edges','No prune')
plt.subplot(1, 3, 2)
dsp.showStdDifference(sgp, sgc, 0.35, 'relative std. difference', None, 'Org<0.35')
plt.subplot(1, 3, 3)
dsp.showStdDifference(sgp, sgc, 0.30, None, None, 'Org<0.30')
plt.tight_layout()
plt.show()


# ------
# information (variance) on each subject

smc, ssc = np.mean(sac, 0), np.std(sac, 0)
smp, ssp = np.mean(sap, 0), np.std(sap, 0)

plt.figure(figsize=[6.4, 2.4])
plt.subplot(1, 2, 1)
plt.imshow((smp-smc)/smc); plt.colorbar()
plt.xlabel('relative difference (mean)')
plt.subplot(1, 2, 2)
sl = ((ssp-ssc)/smc).ravel()
plt.hist(sl, 100, [-0.15, 0.15]); plt.grid(True)
plt.ylabel('# of edges'); plt.xlabel('relative difference (std.)')
plt.tight_layout()
plt.show()


# ------
# dynamic of individual edge

r = [np.random.randint(min(nSub)) for i in range(16)]

i = 41; j = 43
dsp.drawDynamicGrid(dfcc, r, i, j); plt.show()

# use the discriminative edges: elistw, and eliste
pruneTh = 0.3; th = 0.2
elistw, eliste = anl.pickTopEdges(anl.setCond(dm, np.abs(mc) <= pruneTh), [-th, th])
kw = 0; ke = 0
i, j = elistw[kw]
plt.figure(1)
dsp.drawDynamicGrid(dfcc, r, i, j, False, True, True)
plt.figure(2)
dsp.drawDynamicGrid(dfcp, r, i, j, False, True, True)
plt.show()

# ------
# co-appearance of edges

r = [np.random.randint(min(nSub)) for k in range(9)]
dsp.showCoDynamicGrid(dfcp, mgc, r, [eliste[0], elistw[0]], True, True, 3); plt.show()
dsp.showCoDynamicGrid(dfcp, mgc, r, eliste[0:4:2] + elistw[0:4:2], nRow=3); plt.show()

r = [np.random.randint(min(nSub)) for k in range(20)]
dsp.showCoDynamicGrid(dfcp, mgc, r, [eliste[0], elistw[0], elistw[2]], True, True, 4); plt.show()

k = 0
i1, j1 = eliste[0]
i2, j2 = elistw[0]

# coc = anl.getCoChangeDirect([dfcp[k][:,i1,j1], dfcp[k][:,i2,j2]], [mgc[i1,j1], mgc[i2,j2]])
coc = anl.getCoChange(dfcp, mgc, [eliste[0], elistw[0]])
np.mean(abs(coc))

# cot = anl.getCoTrendDirect([dfcp[k][:,i1,j1], dfcp[k][:,i2,j2]])
cot = anl.getCoTrend(dfcp, [eliste[0], eliste[0]])
np.mean(abs(cot))


# ------


msea = np.zeros([nSub[0], nNode, nNode])
for k in range(nSub[0]):
    print(k)
    msea[k] = anl.getMSEToNormal(dfcc[k], mac[k], sac[k])

mseg = np.zeros([nNode, nNode])
for ii in range(nNode):
    for jj in range(nNode):
        temp = anl.pickEdgeOfAll(dfcc, ii, jj)
        mseg[ii][jj] = anl.getMSEToNormal_one(temp, mgc[ii][jj], sgc[ii][jj])

with open(DATAPREFIX+'/analysis/mse-p.pickle','wb') as f:
    pickle.dump([msea, mseg], f)

with open(DATAPREFIX + '/analysis/mse-p.pickle', 'rb') as f:
    msea, mseg = pickle.load(f)


plt.subplot(3, 3, 1)
plt.imshow(mseg); plt.colorbar(); plt.title('MSE on snapshots')
# plt.show()

for k in range(2, 10):
    plt.subplot(3, 3, k)
    k = np.random.randint(nSub[0])
    plt.imshow(msea[k]); plt.colorbar(); plt.title('MSE on sub-'+str(k))
plt.tight_layout()
plt.show()

# fitness to gaussian (each subject)
r = [np.random.randint(nSub[0]) for i in range(9)]
for k in range(9):
    plt.subplot(3, 3, k + 1)
    dsp.drawDistriNorm(dfcc[r[k]][:, i, j], 20, mgc[i, j], sgc[i, j])
plt.show()
# Result: not very similar (too little samples on each subject)

# fitness to gaussian (each subject)
