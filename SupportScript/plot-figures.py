# -*- coding: utf-8 -*-
"""
Created on Thu Oct 27 22:00:39 2016

@author: Tian
"""

import sys,os
import numpy as np
import matplotlib.pyplot as plt

def main():
    pass


if __name__=='__main__':
    if len(sys.argv)<4 or len(sys.argv)>5:
        print('Usage: <type> <summary file> <output file> [prefix of test result files (def: tst)]')
        exit()
    main(smyType, smyFile, outputFn, tstPrefix)

