# -*- coding: utf-8 -*-
"""
Created on Tue Nov  8 09:33:10 2016

@author: Tian
"""

import sys,re


def main(finpath, foutpath, showHead):
    pat=re.compile(
        r'.*/?p-s\d+-\d+/graph-[\d\.]+/([.\d]+)/diff-([\d\.]+)-(0.\d+)\.txt:  \# of edges: (\d+)')
    rep=r'\1\t\3\t\2\t\4'
    head='theta\tminsup\talpha\tnmotif'
    with open(finpath, 'r') as fin:
        with open(foutpath, 'w') as fout:
            if showHead:
                fout.write(head)
                fout.write('\n');
            for line in fin:
                l=pat.sub(rep,line)
                fout.write(l)
        

if __name__=='__main__':
    if len(sys.argv)<3 or len(sys.argv)>4:
        print('Usage: <in-file path> <out-file path> [showHead=0/1]')
        exit()
    finpath=sys.argv[1]
    foutpath=sys.argv[2]
    showHead=False;
    if len(sys.argv)==4:
        showHead=(int(sys.argv[3])==1)
    main(finpath, foutpath, showHead)


