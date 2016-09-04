# -*- coding: utf-8 -*-
"""
Created on Sat Aug 13 22:42:08 2016

@author: Tian Zhou
"""

import os,sys

def processOneFile(filename, content):
    cnt=0;
    accuracy=0.0
    precision=0.0
    recall=0.0
    f1=0.0
    fp=0.0
    fn=0.0
    sd=0.0
    sr=0.0
    with open(filename) as fin:
        header=fin.readline()[:-1].split('\t')
        pACC=header.index('accuracy')
        pPRE=header.index('precision')
        pREC=header.index('recall')
        pF1=header.index('f1')
        pfp=header.index('fre-pos')
        pfn=header.index('fre-neg')
        psd=header.index('score-diff')
        psr=header.index('score-ratio')
        for line in fin:
            line=line[:-1].split('\t')
            accuracy+=float(line[pACC])
            precision+=float(line[pPRE])
            recall+=float(line[pREC])
            f1+=float(line[pF1])
            fp+=float(line[pfp])
            fn+=float(line[pfn])
            sd+=float(line[psd])
            sr+=float(line[psr])
            cnt+=1
    if cnt!=0:
        accuracy/=cnt
        precision/=cnt
        recall/=cnt
        f1/=cnt
        fp/=cnt
        fn/=cnt
        sd/=cnt
        sr/=cnt
    return (cnt, accuracy, precision, recall, f1, fp, fn, sd, sr)

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
    header=['filename','count','accuracy','precision','recall','f1','fre-pos','fre-neg','score-diff','score-ratio']
    data={}
    print('loading')
    for fn in os.listdir(folder):
        if fn.startswith('res-') and fn.endswith('.txt') and os.path.isfile(folder+fn):
            print('  loading: '+fn)
            data[fn]=processOneFile(folder+fn, header)
    print('loaded: '+str(len(data)))
    print('outputting')
    output(outFn, data, header, nDig)
    print('done')
    

if __name__=='__main__':
    if len(sys.argv)<3 or len(sys.argv)>4:
        print('usage: <folder of test result files> <output file name> [# of digit for result (def: 3)]')
        print('\tFile names should start with "res-" and end with ".txt"')
        exit()
    folder=sys.argv[1]
    outFn=sys.argv[2]
    nDig=int(sys.argv[3]) if len(sys.argv)==4 else 3
    main(folder,outFn,nDig)
