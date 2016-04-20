import numpy as np
import matplotlib.pyplot as plt
import argparse as ap
from collections import namedtuple

parser = ap.ArgumentParser(description = "Read the results of a bout between two chess engines and plot some stats.")
parser.add_argument("result_file", type=ap.FileType('r'), help = "A file of the form produced by test_katyusha.py")
parser.add_argument("--name1", help="Name of First Engine", default = "Engine 1")
parser.add_arguemnt("--name2", help = "Name of Second Engine", default = "Engine 2")
args = parser.parse_args()

f = args.result_file
#Print the descriptions of the engines
for i in xrange(2): print f.readline()

Games = namedtuple('Games', ['won', 'drew', 'lost'])
n_game = 0

#when first player is white
white_res = np.array([0,0,0])
#when first player is black
black_res = np.array([0,0,0])
res = None
while not f.readline() == "":
    #determine who is what
    l = f.readline()
    score = float(f.readline().strip().split()[-1])
    score_ind = int(2*score)
    n_game += 1
#    print l, score
    if "White" in l:
        #engine 2 is white
        black_res[score_ind] += 1
    else:
        white_res[2-score_ind] += 1

print "First Engine White", white_res
print "Frist Engine Black ", black_res
res = white_res + black_res

fig, ax = plt.subplots()
plt.title(args.name1+" vs "+args.name2)
ax.bar(np.arange(3), res)
ax.set_xticklabels(("Won", "Drew", "Lost"))
plt.show()
