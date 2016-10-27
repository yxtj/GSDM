# -*- coding: utf-8 -*-
"""
Created on Thu Oct 27 00:36:01 2016

@author: Tian
"""

import sys
import re

TYPE_STR='<type> should be one of: func, cut, dug'

def _procCommon(header, data, headHeader, regPat, regRep):
    newHeader=headHeader+header;
    newData=[]
    cr=re.compile(regPat)
    for line in data:
        nl=cr.sub(regRep, line)
        newData.append(nl)
    return (newHeader, newData)
    
def _checkMatch(pat, line):
    if not re.match(pat, line):
        raise Exception('Cannot match content of the summary file. '
        'Please check the prefix.')

def procFunc(header, data, tstPrefix):
    # tst-0.4-diff-1-0.3.txt
    # tst-(0.\d+)-diff-(\d(?:\.\d)?)-(0\.\d+)\.txt
    _regPattern=tstPrefix+r'-(0.\d+)-diff-(\d(?:\.\d)?)-(0\.\d+)\.txt'
    _regReplace=r'\1\t\3\t\2'
    _headHeader='theta\tminsup\talpha\t'+'num\t'
    _checkMatch(_regPattern, data[0])
    return _procCommon(header,data, _headHeader, _regPattern, _regReplace)
    

def procCut(header, data, tstPrefix):
    # res-0.2-24-0.1-0.1.txt
    _regPattern=tstPrefix+r'-(0.\d+)-(\d)(\d+)?-(0\.\d+)-(0\.\d+)\.txt'
    _regReplace=r'\1\t\4\t\5\t\2\t\3'
    _headHeader='theta\tf-pos\tf-neg\tn-min\tn-max\t'+'num\t'
    _checkMatch(_regPattern, data[0])
    return _procCommon(header,data, _headHeader, _regPattern, _regReplace)
    
def procDug(header, data, tstPrefix):
    # res-0.03.txt
    _regPattern=tstPrefix+r'-(0.\d+)\.txt'
    _regReplace=r'\1'
    _headHeader='thres\t'+'num\t'
    _checkMatch(_regPattern, data[0])
    return _procCommon(header,data, _headHeader, _regPattern, _regReplace)

def output(fn, header, data):
    with open(fn,'w') as f:
        f.write(header)
        for line in data:
            f.write(line)

def main(smyType, smyFile, outputFn, tstPrefix):
    if 'func'==smyType:
        pFun=procFunc;
    elif 'cut'==smyType:
        pFun=procCut;
    elif 'dug'==smyType:
        pFun=procDug;
    else:
        print('ERROR: '+TYPE_STR)
        exit()
    with open(smyFile, 'r') as f:
        # '\n' is kept at the end of each line
        header=f.readline()
        data=f.readlines()
        f.close()
        (newHeader,newData)=pFun(header,data,tstPrefix)
        output(outputFn, newHeader, newData)

if __name__=='__main__':
    if len(sys.argv)<4 or len(sys.argv)>5:
        print('Usage: <type> <summary file> <output file> [prefix of test result files (def: tst)]')
        print('\t'+TYPE_STR)
        exit()
    smyType=sys.argv[1]
    smyFile=sys.argv[2]
    outputFn=sys.argv[3]
    tstPrefix='tst'
    if len(sys.argv)==5:
        tstPrefix=sys.argv[4]
    main(smyType, smyFile, outputFn, tstPrefix)

