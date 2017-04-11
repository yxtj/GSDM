import DataLoader as dl
import numpy as np

class CorrLoader:
    def __init__(self, path):
        self.path=path

    '''
    Return a matrix (2D-array).
    '''
    def loadOne(self, fn):
        f = open(self.path+'/'+fn,'r')
        data = f.read()
        # assume non-compressed
        data = data.split('\n')
        n = int(data[0])
        res = []
        for line in data[1:]:
            t = []
            for v in line.split(' '):
                if v:
                    t.append(np.float32(v))
            if len(t) != 0:
                res.append(t)
        f.close()
        return np.array(res, dtype='float32')

    '''
    Return a list (file) of matrices. Special case: a file per subject 
    '''
    def loadWhole(self, types):
        l = dl.getFileNames(self.path, types)
        res = []
        for fn in l:
            t = self.loadOne(fn)
            res.append(t)
        return np.array(res, dtype='float32')

    '''
    Return a list (subject) of matrices
    '''
    def loadDynamic(self, types):
        l = dl.getFileNames(self.path, types)
        l = dl.sortUpWithSubject(l)
        res = []
        for sub in l:
            t = sum(self.loadOne(fn) for fn in sub) / len(sub)
            res.append(t)
        return np.array(res, dtype='float32')

    '''
    Return the a list (subject) of list (snapshot) of matrices
    '''
    def loadSnapshots(self, types, nSub : int = None, minSeq : int = None):
        l = dl.getFileNames(self.path, types)
        l = dl.sortUpWithSubject(l, nSub, minSeq)
        res = []
        for sub in l:
            t = [self.loadOne(fn) for fn in sub]
            res.append(np.array(t, dtype='float32'))
        return res
