#!/usr/bin/env python3

import os
from utils import check_convergence

mnt1 = os.environ['MNT1']
mnt2 = os.environ['MNT2']

path1 = os.path.join(mnt1, 'pif')
path2 = os.path.join(mnt2, 'pif')

def write_serial(path1, path2):
    import time
    time.sleep(1)
    with open(path1, 'w') as f1:
        f1.write('bite')
    time.sleep(1)
    with open(path2, 'w')  as f2:
        f2.write('pif')
    time.sleep(1)

try:
    write_serial(path1, path2)
    check_convergence(path1, path2)
    print(write_serial.__name__, "passes")
except Exception as err:
    print("test '%s'" % write_serial.__name__, "failed:", str(err))

