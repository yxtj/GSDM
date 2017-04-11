import os, re
from functools import total_ordering
from typing import Union, List, Tuple

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

    def valid(self):
        return hasattr(self, 'id')

    @staticmethod
    def valid(fn):
        return SubjectInfo.fnReg.match(fn) != None

    def makeFileName(self):
        return str(self.type) + '-' + self.id + '-' + str(self.seq) + '.txt'

    def __bool__(self):
        return self.valid()

    def __str__(self):
        return self.makeFileName() if self.valid() else ''

    def __eq__(self, other):
        return self.id == other.id and self.seq == other.seq

    def __lt__(self, other):
        return self.id < other.id or (self.id == other.id and self.seq < other.seq)

    def __hash__(self):
        return hash(self.id, self.seq)


def getFileNames(path, types, nFile = None):
    if not isinstance(types, list):
        types = [types]
    l = os.listdir(path)
    if isinstance(nFile, int):
        l = l[:nFile]
    res = []
    for fn in l[:nFile]:
        m = SubjectInfo.fnReg.match(fn)
        if m and int(m.group(1)) in types:
            res.append(fn)
    return res


def sortUpWithSubject(fnlist, maxnsub : int = None, nseq : Union[int, List[int], Tuple[int, int]] = None):
    subLimit = maxnsub if isinstance(maxnsub, int) else len(fnlist)
    seqMin = 0; seqMax = len(fnlist)
    if isinstance(nseq, int):
        seqMin = nseq
    elif isinstance(nseq, list) or isinstance(nseq, tuple):
        seqMin = nseq[0]
        if len(nseq) > 1:
            seqMax = nseq[1]

    infoList = []
    for fn in fnlist:
        infoList.append(SubjectInfo(fn))
    sorted(infoList)
    res = [[infoList[0].makeFileName()]]
    for p in range(1, len(infoList)):
        if infoList[p - 1].id == infoList[p].id:
            res[-1].append(infoList[p].makeFileName())
        else:
            subLimit -= 1
            if subLimit <= 0:
                break
            res.append([infoList[p].makeFileName()])
    if seqMin != 0 or seqMax != len(fnlist):
        i = 0
        while i < len(res):
            if seqMin <= len(res[i]) <= seqMax:
                i += 1
            else:
                del res[i]
    return res
