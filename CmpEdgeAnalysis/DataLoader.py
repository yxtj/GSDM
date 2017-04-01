import os,re
from functools import total_ordering


@total_ordering
class SubjectInfo:
    fnReg = re.compile(r"(\d+)-(.+?)-(\d+)\.txt")

    def __init__(self, fn):
        self.parseFileName(fn)

    def parseFileName(self, fn):
        m = SubjectInfo.fnReg.match(fn)
        if m:
            self.type = int(m.group(1))
            self.id = m.group(2)
            self.seq = int(m.group(3))

    def makeFileName(self):
        return str(self.type) + '-' + self.id + '-' + str(self.seq) + '.txt'

    def __eq__(self, other):
        return self.id == other.id and self.seq == other.seq

    def __lt__(self, other):
        return self.id < other.id or (self.id == other.id and self.seq < other.seq)

    def __hash__(self):
        return hash(self.id, self.seq)


def getFileNames(path, types):
    if not isinstance(types, list):
        types = [types]
    l = os.listdir(path)
    res = []
    for fn in l:
        m = SubjectInfo.fnReg.match(fn)
        if m and int(m.group(1)) in types:
            res.append(fn)
    return res


def sortUpWithSubject(fnList):
    infoList = []
    for fn in fnList:
        infoList.append(SubjectInfo(fn))
    sorted(infoList)
    res = [[infoList[0].makeFileName()]]
    for p in range(1, len(infoList)):
        if infoList[p - 1].id == infoList[p].id:
            res[-1].append(infoList[p].makeFileName())
        else:
            res.append([infoList[p].makeFileName()])
    return res
