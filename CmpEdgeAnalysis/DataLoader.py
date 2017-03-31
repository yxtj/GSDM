import os,re

__fn_reg=re.compile(r"(\d+)-(.+?)-(\d+)\.txt")


def getFileNames(path, types):
    if not isinstance(types, list):
        types = [types]
    l = os.listdir(path)
    res = []
    for fn in l:
        m = __fn_reg.match(fn)
        if m and int(m.group(1)) in types:
            res.append(fn)
    return res

