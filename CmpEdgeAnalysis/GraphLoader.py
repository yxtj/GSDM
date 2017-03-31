import DataLoader as dl


class GraphLoader:
    def __init__(self, path):
        self.path=path

    @staticmethod
    def checkCompressLevel(mb):
        # input: magic btype
        # output: (type, offset)
        return (0,0)

    @staticmethod
    def __loadOneText(data):
        data=data.split('\n')
        n = int(data[0])
        res = []
        for line in data[1:]:
            t = []
            for v in line.split(' ')[1:]:
                t.append(int(v))
            res.append(t)
        return res

    def loadOne(self, fn):
        f=open(self.path+'/'+fn,'r')
        data=f.read()
        f.close()
        (cl, off)=GraphLoader.checkCompressLevel(data[0])
        # assume non-compressed
        return GraphLoader.__loadOneText(data[off:])

    def load(self, types):
        l=dl.getFileNames(self.path, types)
        res=[]
        for fn in l:
            t=self.loadOne(fn)
            res.append(t)
        return res
