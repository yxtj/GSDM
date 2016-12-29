# -*- coding: utf-8 -*-
"""
Created on Tue Nov 15 15:21:34 2016

@author: Tian
"""

import sys,os
import re
import random
import math
import shutil

__MTHD_RANODM='random'
__MTHD_CUT='cut'

MTHD_LIST=[__MTHD_RANODM, __MTHD_CUT]

# return 3-level list.
# 1-level: type-list, 2-level: subject-list, 3-level: filename-list
def loadFileList(inDir, pattern):
    d={}
    for fn in os.listdir(inDir):
        m=pattern.match(fn)
        if m:
            tp=m.group('type')
            id=m.group('id')
            key=(tp,id)
            v=d.get(key)
            if v:
                v.append(fn)
            else:
                d[key]=[fn]
    # prepare a dict of type-(id,index)
    lkey=list(d.keys())
    lval=list(d.values())
    d2={}
    for i in range(len(lkey)):
        (tp,id)=lkey[i]
        v=d2.get(tp)
        if v:
            v.append((id,i))
        else:
            d2[tp]=[(id,i)]
    # prepare the output list
    res=[]
    for tp in d2:
        d2[tp].sort()
        temp=[lval[i] for (id,i) in d2[tp]]
        res.append(temp)
    return res

def parseMethod(method, nParts):
    if method.find(__MTHD_RANODM)!=-1:
        name=__MTHD_RANODM
        param=123456
        p=method.find('-')
        if p!=-1:
            param=int(method[p+1:])
    elif method.find(__MTHD_CUT)!=-1:
        name=__MTHD_CUT
        param=None
    return (name,param)

def getIndexesForEachPart(mName, mParam, portions, nType, nSub):
    nDir=len(portions)
    res=[ [ None for j in range(nType) ] for i in range(nDir) ]
    for it in range(nType):
        l=list(range(nSub[it]))
        if mName==__MTHD_RANODM:
            random.seed(mParam)
            for i in range(nSub[it]):
                a=random.randint(0,nSub[it]-1)
                b=random.randint(0,nSub[it]-1)
                (l[a], l[b])=(l[b], l[a])
        elif mName==__MTHD_CUT:
            # nothing needed to be done for this method
            pass
        last=0
        for id in range(nDir):
            pos=int(math.ceil(nSub[it] * sum(portions[0:id+1])))
            res[id][it]=l[last:pos]
            last=pos
    return res

def main(inDir, method, pattern, portions, outDirs):
    nDir=len(outDirs)
    (mName, mParam)=parseMethod(method,nDir)
    print('Getting file lists...')
    # level 1: type-list, level 2: subject-list, level 3: filename-list
    flist=loadFileList(inDir, pattern)
    nType=len(flist)
    print('  # of valid types: '+str(nType))
    nSub=[len(l) for l in flist]
    print('  # of valid subjects: '+str(sum(nSub)))
    print('  # of valid files: '+str(
        sum(sum(len(l2) for l2 in l1) for l1 in flist) ))

    print('Generating output mapping...')
    # level 1: dir-list, level 2: type-list, level 3: subject-index-list
    idxs=getIndexesForEachPart(mName, mParam, portions, nType, nSub)
    print('  # of subjects in each output dir: '+str(
        [sum(len(ltype) for ltype in ldir) for ldir in idxs] ))

    print('Copying files...')
    if not inDir.endswith('/') and not inDir.endswith('\\'):
        inDir+='/'
    for i in range(nDir):
        print('  processing '+str(i+1)+'-th output')
        od=outDirs[i]
        if not od.endswith('/') and not od.endswith('\\'):
            od+='/'
        if not os.path.isdir(od):
            os.makedirs(od)
        print('    # of subjects in each type: '+str([len(t) for t in idxs[i]]))
        for j in range(nType):
            for idx in idxs[i][j]:
                for fn in flist[j][idx]:
                    shutil.copy2(inDir+fn, od+fn)
    print('Finish')


if __name__ == '__main__':
    if(len(sys.argv)<6):
        print('usage: <input folder> <method> <output folder 1> <portion 1> <output folder 2> <portion 2> ... [-pattern:<cared name pattern>]')
        print('  <method>: method of selecting the files. Supports:')
        print('    ['+__MTHD_RANODM+']: randomly assign the list. can customize the seed with "random-<seed>", 123456 by default')
        print('    ['+__MTHD_CUT+']: cut the sorted list by type. guarantee the portion on each type')
        print('  [cared name pattern]: customize the filename pattern using EMACScript grammer (mind to use escape in shell for "\\"). The part in "()" is the key which is used for dividing. "(?P<type>\\d+)-(?P<id>\\d+)-\\d+\\.txt" by default')
        sys.exit()
    inDir=sys.argv[1]
    method=sys.argv[2]
    pattern=r'(?P<type>\d+)-(?P<id>\d+)-\d+\.txt'
    _pEnd=len(sys.argv)
    if sys.argv[-1].find('-pattern:')==0:
        pattern=sys.argv[-1]
        _pEnd-=1
    outDirs=[]
    portions=[]
    for i in range(3, _pEnd, 2):
        outDirs.append(sys.argv[i])
        portions.append(float(sys.argv[i+1]))
    print('intput folder: '+inDir)
    print('method: '+method)
    if method not in MTHD_LIST:
        print('ERROR: the method is not supported')
        exit()
    print('name pattern: '+pattern)
    print('# of output folders: '+str(len(outDirs)))
    print('  their portions are: '+str(portions))
    if not os.path.isdir(inDir):
        print('ERROR: the input folder is not a valid folder')
        exit()
    if sum(portions)!=1:
        print('ERROR: the summation of the portions does not equal to 1!')
        exit()

    pattern=re.compile(pattern)
    main(inDir, method, pattern, portions, outDirs)
