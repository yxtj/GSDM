import sys, os
import DataLoader as dl
import numpy as np
import numba as nb
from prepare import *
import GraphLoader as gl
from typing import Union, List, Tuple


@nb.jit(nopython=True)
def get_type_count(n: int, subs: List[List]):
    cnt = np.zeros([n, n])
    for sub in subs:
        for g in sub:
            # x, y = g.nonzero()
            # cnt[x, y] += 1
            #cnt[[v[0] for v in g], [v[1] for v in g]] += 1
            for i in range(n):
                cnt[i, g[i]] += 1
    return cnt


def find_label(cnt_w: np.ndarray, cnt_e: np.ndarray):
    assert cnt_w.shape == cnt_e.shape
    label = np.zeros_like(cnt_w, np.int8)
    label[cnt_w > cnt_e] = -1
    label[cnt_e > cnt_w] = 1
    return label


def output_label(fn: str, label: np.ndarray):
    with open(fn, 'w') as f:
        f.write(str(len(label)))
        f.write('\n')
        for line in label:
            t = ' '.join([str(v) for v in line])
            f.write(t)
            f.write('\n')


@nb.jit(nopython=True)
def merge_graph(n, graph_w, graph_e, label):
    res = [[] for i in range(n)]
    for i in range(n):
        lb = label[i]
        lw = np.array(graph_w[i], int)
        le = np.array(graph_e[i], int)
        res[i].extend(lw[lb[lw] == -1])
        res[i].extend(le[lb[le] == 1])
        res[i].sort()
    return res


def writeGraph(fn, g):
    n = len(g)
    with open(fn, 'w') as f:
        f.write(str(n))
        f.write('\n')
        for i in range(n):
            f.write(str(i))
            f.write('\t')
            temp = ' '.join(str(t) for t in g[i])
            f.write(temp)
            if len(temp) != 0:
                f.write(' ')
            f.write('\n')


def main(path_weakened: str, path_enhanced: str, path_out: str):
    print('Loading enhanced graphs...')
    gLoader = gl.GraphLoader(path_weakened)
    sub_w = gLoader.load_as_subjects(None)  # all types
    assert len(sub_w) != 0
    node = len(sub_w[0][0])
    cnt_w = get_type_count(node, sub_w)
    print('Loading weakened graphs...')
    gLoader = gl.GraphLoader(path_enhanced)
    sub_e = gLoader.load_as_subjects(None)
    cnt_e = get_type_count(node, sub_e)

    print('Generating and dumping the label matrix...')
    if not os.path.exists(path_out):
        os.makedirs(path_out)
    label = find_label(cnt_w, cnt_e)
    output_label(path_out + '/label.txt', label)

    print('Merging graphs...')
    files = dl.getFileNames(path_weakened, None)
    files_sub = dl.sortUpWithSubject(files, None)
    print(len(sub_w), len(sub_e), len(files_sub))
    print(len(sub_w[0]), len(sub_e[0]), len(files_sub[0]))
    for i in range(len(files_sub)):
        for j in range(len(files_sub[i])):
            g = merge_graph(node, sub_w[i][j], sub_e[i][j], label)
            writeGraph(path_out + '/' + files_sub[i][j], g)


if __name__ == '__main__':
    argc = len(sys.argv)
    if argc < 4:
        print('Merge enhanced-only and weakened-only graphs by choose the most frequent type for each edge')
        print('Usage: <path-weakened> <path-enhanced> <path-out>\n'
              '  <path-weakened>: the folder for weakened-only graphs.\n'
              '  <path-enhanced>: the folder for enhanced-only graphs.\n'
              '  <path-out>: the folder for output:\n'
              '    a "label.txt" file stores the label of each edge.')
        exit()
    path_w = sys.argv[1]
    path_e = sys.argv[2]
    path_out = sys.argv[3]
    main(path_w, path_e, path_out)
