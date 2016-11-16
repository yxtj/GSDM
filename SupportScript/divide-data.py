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

# return [[],[]]
def loadFileList(inDir, pattern):
    d={}
    for fn in os.listdir(inDir):
        m=pattern.match(fn)
        if m:
            key=m.group(1)
            v=d.get(key)
            if v:
                v.append(fn)
            else:
                d[key]=[fn]
    # sort the file list according to the key
    lkey=list(d.keys())
    lval=list(d.values())
    idx=sorted(range(len(lkey)), key=lambda k:lkey[k])
    res=[lval[i] for i in idx]
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
        param=[]
        p=method.find('-')
        if p==-1:
            each=1.0/nParts
            param=[i*each for i in range(1,nParts)]
        else:
            plast=p+1
            p=method.find('-',plast)
            while p!=-1:
                param.append(float(method[plast:p]))
                plast=p+1
                p=method.find('-',plast)
            param.append(float(method[plast:]))
            if len(param)+1 != nParts:
                raise ValueError('number of outputs does not match cutting points')
    return (name,param)

def getIndexesForEachPart(mName, mParam, portions, n):
    l=list(range(n))
    if mName==__MTHD_RANODM:
        random.seed(mParam)
        for i in range(n):
            a=random.randint(0,n-1)
            b=random.randint(0,n-1)
            (l[a], l[b])=(l[b], l[a])
    elif mName==__MTHD_CUT:
        # nothing needed to be done here
        pass
    res=[ [] for i in range(n) ]
    last=0
    for i in range(len(portions)):
        pos=math.ceil(n* sum(portions[0:i+1]))
        res[i]=l[last:pos]
        last=pos
    return res

def main(inDir, method, pattern, portions, outDirs):
    (mName, mParam)=parseMethod(method,len(outDirs))
    print('Getting file lists...')
    flist=loadFileList(inDir, pattern)
    print('  # of valid groups:', len(flist))
    print('  # of valid files in all:', sum(len(l) for l in flist))

    print('Generating output mapping...')
    idxs=getIndexesForEachPart(mName, mParam, portions, len(flist))
    print('  # of groups in each output dir: '+str([len(l) for l in idxs]))

    print('Copying files...')
    if not inDir.endswith('/') and not inDir.endswith('\\'):
        inDir+='/'
    for i in range(len(outDirs)):
        print('  processing '+str(i)+'-th output')
        od=outDirs[i]
        if not od.endswith('/') and not od.endswith('\\'):
            od+='/'
        if not os.path.isdir(od):
            os.makedirs(od)
        for idx in idxs[i]:
            for fn in flist[idx]:
                shutil.copy2(inDir+fn, od+fn)
    print('Finish')


if __name__ == '__main__':
    if(len(sys.argv)<6):
        print('usage: <input folder> <method> <output folder 1> <portion 1> <output folder 2> <portion 2> ... [-pattern:<cared name pattern>]')
        print('  <method>: method of selecting the files. Supports:')
        print('    ['+__MTHD_RANODM+']: randomly assign the list. can customize the seed with "random-<seed>", 123456 by default')
        print('    ['+__MTHD_CUT+']: cut the sorted list sequentially')
        print('  [cared name pattern]: customize the filename pattern using EMACScript grammer (mind to use escape in shell for "\\"). The part in "()" is the key which is used for dividing. "\\d+-(\\d+)-\\d+\\.txt" by default')
        sys.exit()
    inDir=sys.argv[1]
    method=sys.argv[2]
    pattern=r'\d+-(\d+)-\d+\.txt'
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
