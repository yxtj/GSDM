import os, sys, re
from math import inf


def countUndirEdgeOnFile(fn) -> (int, int):
    cnt = 0
    n = 0
    with open(fn) as f:
        line = f.readline()
        n = int(line)
        for line in f:
            pf = line.find('\t')
            id = line[:pf]  # is a string
            pl1 = line.rfind('\n')
            pl2 = line.rfind('\r')
            pl = max(pl1 if pl1 != -1 else inf, pl1 if pl1 != -1 else inf)
            l = line[pf + 1:pl].split(' ') # strings
            cnt += len(l)
            if id in l: # remove self loop
                cnt -= 1
    return cnt, n


def main(path: str, types: list, dir: bool):
    l = os.listdir(path)
    pat = re.compile(r'(\d)-.+-\d+\.txt')
    if types:
        temp = []
        for fn in l:
            m = pat.match(fn)
            if m and int(m.group(1)) in types:
                temp.append(fn)
        l = temp
    print('total graphs:', len(l))
    if len(l) ==0:
        print('no valid graph files!')
        exit()
    cnt = 0
    nNode = 0
    nFile = len(l)
    for fn in l:
        temp = countUndirEdgeOnFile(path + '/' + fn)
        cnt += temp[0]
        nNode = temp[1]

    print('total edges:', cnt)
    print('averaged edges on each graph:', cnt/nFile)
    print('averaged degree:', cnt / nFile / nNode)
    total = nFile * (nNode ** 2 - nNode)  # remove self loop
    print('averaged density:', cnt/total)


if __name__ == '__main__':
    if len(sys.argv) <= 1:
        print('Count the edges of graphs in a folder.\n'
              'Usage: <path> [type] [dir]\n'
              '  <path>: the folder to the graph files.\n'
              '  <type>: (=all) the types to consider, use "," to separate multiple types\n'
              '  <dir>: (=False) whether the graphs are directed graph.')
        exit()
    # path = '../data_abide/data-all/dis-1/enhanced'
    path = sys.argv[1]
    types = None
    if len(sys.argv) > 2:
        types = sys.argv[2]
        if types.lower() != 'all':
            types = [int(v) for v in types.split(',')]
    # dir = sys.argv[3].lower() in {'1', 'true', 'yes'} if len(sys.argv) > 3 else False
    dir = False
    main(path, types, dir)
