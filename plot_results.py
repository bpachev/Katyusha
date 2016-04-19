import numpy as np
import matplotlib.pyplot as plt
import argparse as ap
from collections import namedtuple

parser = ap.ArgumentParser(description = "Read the results of a bout between two chess engines and plot some stats.")
parser.add_argument("result_file", type=ap.FileType('r'), help = "A file of the form produced by test_katyusha.py")

args = parser.parse_args()

f = args.result_file
#Print the descriptions of the engines
for i in xrange(2): print f.readline()

Games = namedtuple('Games', ['won', 'drew', 'lost'])
n_game = 0

#when first player is white
white_res = [0,0,0]
#when first player is black
black_res = [0,0,0]
res = None
while not f.readline() == "":
    #determine who is what
    l = f.readline()
    score = float(f.readline().strip().split()[-1])
    score_ind = int(2*score)
    print l, score
    if "White" in l:
        #engine 2 is white
        black_res[score_ind] += 1
    else:
        white_res[2-score_ind] += 1

print black_res, white_res
