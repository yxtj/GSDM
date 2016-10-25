# -*- coding: utf-8 -*-
"""
Created on Sat Aug 13 22:42:08 2016

@author: Tian Zhou
"""

import os,sys

def processOneFile(filename, title):
    ln=len(title)
    cnt=0
    cntInvalid=[0 for i in range(ln)]
    result=[0.0 for i in range(ln)]
    idx=[-1 for i in range(ln)]
    with open(filename) as fin:
        header=fin.readline()[:-1].split('\t')
        for i in range(ln):
            idx[i]=header.index(title[i])
        for line in fin:
            items=line[:-1].split('\t')
            invalid=False
            for i in range(ln):
                try:
                    v=float(items[idx[i]])
                except:
                    v=0
                    cntInvalid[i]+=1
                    invalid=True
                result[i]+=v
            cnt+=1
            if invalid:
                print('find invalid number at line: '+line)
    if cnt!=0:
        for i in range(ln):
            result[i]/=(cnt-cntInvalid[i])
    result.insert(0,cnt)
    return result

def output(outFn, data, header, nDig):
    fout=open(outFn, 'w')
    if not fout:
        print('cannot open output file')
        return
    pointFmt='%.'+str(nDig)+'f'
    fout.write('\t'.join(header)+'\n')
    for fn,t in sorted(data.items()):
        fout.write(fn+'\t'+'\t'.join([str(v) if isinstance(v,int) else pointFmt%v for v in t]))
        fout.write('\n')

def main(folder, outFn, nDig):
    if not folder.endswith('\\') and not folder.endswith('/'):
        folder+='/'
    header=['accuracy','precision','recall','f1','fre-pos','fre-neg','score-diff','score-ratio']
    data={}
    print('loading')
    for fn in os.listdir(folder):
        if fn.startswith('tst-') and fn.endswith('.txt') and os.path.isfile(folder+fn):
            print('  loading: '+fn)
            data[fn]=processOneFile(folder+fn, header)
    print('loaded: '+str(len(data)))
    print('outputting')
    output(outFn, data, header, nDig)
    print('done')
    

if __name__=='__main__':
    if len(sys.argv)<3 or len(sys.argv)>4:
        print('usage: <folder of test result files> <output file name> [# of digit for result (def: 3)]')
        print('\tFile names should start with "tst-" and end with ".txt"')
        exit()
    folder=sys.argv[1]
    outFn=sys.argv[2]
    nDig=int(sys.argv[3]) if len(sys.argv)==4 else 3
    main(folder,outFn,nDig)
