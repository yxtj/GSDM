# -*- coding: utf-8 -*-
"""
Created on Sat Aug 13 22:42:08 2016

@author: Tian Zhou
"""

import os,sys

def processOneFile(fn):
    cnt=0;
    accurancy=0.0
    recall=0.0
    f1=0.0
    with open(fn) as fin:
        header=fin.readline()[:-1].split('\t')
        pACC=header.index('accurancy')
        pREC=header.index('recall')
        pF1=header.index('f1')
        for line in fin:
            line=line[:-1].split('\t')
            accurancy+=float(line[pACC])
            recall+=float(line[pREC])
            f1+=float(line[pF1])
            cnt+=1
    if cnt!=0:
        accurancy/=cnt
        recall/=cnt
        f1/=cnt
    return (cnt, accurancy, recall, f1)

def output(outFn, data):
    fout=open(outFn, 'w')
    if not fout:
        print('cannot open output file')
        return
    fout.write('\t'.join(['filename','count','accurancy','recall','f1'])+'\n')
    for fn,t in data.items():
        fout.write(fn+'\t'+'\t'.join([str(v) for v in t]))
        fout.write('\n')

def main(folder, outFn):
    if not folder.endswith('\\') and not folder.endswith('/'):
        folder+='/'
    data={}
    print('loading')
    for fn in os.listdir(folder):
        if fn.startswith('res-') and fn.endswith('.txt') and os.path.isfile(folder+fn):
            print('  loading: '+fn)
            data[fn]=processOneFile(folder+fn)
    print('loaded: '+str(len(data)))
    print('outputting')
    output(outFn, data)
    print('done')
    

if __name__=='__main__':
    if len(sys.argv)!=3:
        print('usage: <folder of test result files> <output file name>')
        exit()
    folder=sys.argv[1]
    outFn=sys.argv[2]
    main(folder,outFn)
