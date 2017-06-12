# import numpy as np

class Graph:
    @staticmethod
    def checkCompressLevel(mb):
        # input: magic btype
        # output: (type, offset)
        return (0, 0)

    @staticmethod
    def __load_text(data):
        data = data.split('\n')
        n = int(data[0])
        res = []
        for line in data[1:n + 1]:
            t = []
            for v in line[line.find('\t') + 1:].split(' '):
                if v:
                    t.append(int(v))
            res.append(t)
        return res

    @staticmethod
    def __dump_text(fn, n, g):
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

    def load(self, fn):
        f = open(fn, 'r')
        data = f.read()
        f.close()
        (cl, off) = Graph.checkCompressLevel(data[0])
        # assume non-compressed
        self.g = Graph.__load_text(data[off:])
        self.n = len(data)

    def dump(self, fn, compress_level=0):
        if compress_level == 0:
            self.__dump_text(fn, self.n, self.g)
        else:
            print('Error: unsupported compress level %d for dumping a graph' % compress_level)
