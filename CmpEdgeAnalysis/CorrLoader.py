import DataLoader as dl
import numpy as np

class CorrLoader:
    def __init__(self, path):
        self.path=path

    def loadOne(self, fn):
        f=open(self.path+'/'+fn,'r')
        data=f.read()
        # assume non-compressed
        data=data.split('\n')
        n=int(data[0])
        res=[]
        for line in data[1:]:
            t=[]
            for v in line.split(' '):
                if v:
                    t.append(float(v))
            if len(t)!=0:
                res.append(t)
        f.close()
        return np.array(res)

    def loadWhole(self, types):
        l=dl.getFileNames(self.path, types)
        res=[]
        for fn in l:
            t=self.loadOne(fn)
            res.append(t)
        return res

    def loadDynamic(self, types):
        l = dl.getFileNames(self.path, types)
        l = dl.sortUpWithSubject(l)
        res = []
        for sub in l:
            t = sum(self.loadOne(fn) for fn in sub) / len(sub)
            res.append(t)
        return res
