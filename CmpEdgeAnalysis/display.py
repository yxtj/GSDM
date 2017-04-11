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


def drawEdgeGrid(corra, r, i, j):
    n = len(r)
    size = int(np.ceil(np.sqrt(n)))
    mg = sum(np.mean(snap[:,i,j]) for snap in corra) / len(corra)
    for k in range(n):
        plt.subplot(size, size, k+1)
        l = range(len(corra[r[k]]))
        ml = np.mean(corra[r[k]][:,i,j])
        plt.plot(l, corra[r[k]][:,i,j], l, np.zeros_like(l)+ml, '--', l, np.zeros_like(l)+mg, '-.')
        plt.ylim([-1, 1])
        plt.grid(True)
        plt.title('sub-'+str(r[k]))
    plt.tight_layout()
    #plt.show()


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

