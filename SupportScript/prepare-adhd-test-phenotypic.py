# -*- coding: utf-8 -*-
"""
Created on Fri Aug 12 09:13:38 2016

@author: Tian Zhou
"""

import os,sys
import csv

SITE={
'1': 'Peking_1',
'2': 'Brown',
'3': 'KKI',
'4': 'NeuroIMAGE',
'5': 'NYU',
'6': 'OHSU',
'7': 'Pittsburgh',
'8': 'WashU',
}

def loadGlobalPhenotypicFile(fn):
    res={};
    with open(fn, 'r') as f:
        header=f.readline();   #read the head line
        lh=header[:-1].split(',')
        POS_ID=lh.index('ID')
        POS_DX=lh.index('DX')
        POS_DXS=lh.index('Secondary Dx ')
        POS_AM=lh.index('ADHD Measure')
        POS_AI=lh.index('ADHD Index')
        POS_I=lh.index('Inattentive')
        POS_H=lh.index('Hyper/Impulsive')
        POS_MS=lh.index('Med Status')
        csvin = csv.reader(f, delimiter = ',')
        for row in csvin:
            if row[POS_DX]=='pending':
                continue
            res[row[POS_ID]]=[row[POS_DX], row[POS_DXS],row[POS_AM],row[POS_AI],row[POS_I],row[POS_H],row[POS_MS]]
    return res
    
def processPhenotypicFile(rootData, folder, glPhentp):
    fin=open(rootData+'/'+folder+'/'+folder+'_TestRelease_phenotypic.csv','r')
    fout=open(rootData+'/'+folder+'/'+folder+'_phenotypic.csv','w')
    header=fin.readline()
    fout.write(header)
    lh=header[:-1].split(',')
    if '"ScanDirID"' in lh:
        POS_ID=lh.index('"ScanDirID"')
    elif '"ScanDir ID"' in lh:
        POS_ID=lh.index('"ScanDir ID"')
    else:
        raise Exception('Unsuppoted header of phenotypic file in '+folder)
    POS_DX=lh.index('"DX"')
    POS_DXS=lh.index('"Secondary Dx "')
    POS_AM=lh.index('"ADHD Measure"')
    POS_AI=lh.index('"ADHD Index"')
    POS_I=lh.index('"Inattentive"')
    POS_H=lh.index('"Hyper/Impulsive"')
    POS_MS=lh.index('"Med Status"')
    cnt=0
    for line in fin:
        row=line.split(','); # keep the last '\n'
        id=row[POS_ID]
        if id in glPhentp:
            v=glPhentp[id]
            row[POS_DX]=v[0]
            row[POS_DXS]=v[1]
            row[POS_AM]=v[2]
            row[POS_AI]=v[3]
            row[POS_I]=v[4]
            row[POS_H]=v[5]
            row[POS_MS]=v[6]
            fout.write(','.join(row))
            cnt+=1
        else:
            print('  id: '+str(id)+' is not fount in the global list')
    fin.close()
    fout.close()
    return cnt


def main(phenoFile, rootData):
    print('loading global list')
    glPhento=loadGlobalPhenotypicFile(phenoFile)
    print('loaded '+str(len(glPhento)))
    validSite=SITE.values()
    for fn in os.listdir(rootData):
        if os.path.isdir(rootData+'/'+fn) and fn in validSite:
            print('processing folder: '+fn)
            cnt=processPhenotypicFile(rootData,fn,glPhento)
            pritn('  processed: '+str(cnt))
    print('done')

if __name__ == '__main__':
    if(len(sys.argv)!=3):
        print('usage: <path to test phenotypic file> <root path of data>')
        sys.exit()
    phenoFile=sys.argv[1]
    rootData=sys.argv[2]
    print('global phenotypic file: '+phenoFile)
    print('root path of data folders: '+rootData)
    
    main(phenoFile, rootData)
