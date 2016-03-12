from network_arch import *
from sys import argv, exit

if len(argv) < 3:
  print "usage weightfile outfile"
  exit()
else:
 weights = argv[1]
 outname = argv[2]

model.load_weights(weights)
args = {}
for name in model.nodes:
  node = model.nodes[name]
  args[name+"_weights"], args[name+"_biases"] = node.get_weights()[0], node.get_weights()[1]

np.savez(outname, **args)
