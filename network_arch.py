from keras.models import Sequential, Graph
from keras.layers.core import Dense, Dropout, Activation
from keras.optimizers import SGD,Adagrad
import numpy as np
from sys import argv, exit

if __name__ == "__main__":
 if len(argv) < 2:
   print "usage outfile"
   exit()

model = Graph()

comps = {'global':{}, 'piece':{}, 'square':{}, 'pawn':{}}
comps['global']['output_neurons'] = 20
comps['piece']['output_neurons'] = 32
comps['square']['output_neurons'] = 64
comps['pawn']['output_neurons'] = 20
comps['global']['input_neurons'] = 15 #SIDE TO MOVE, 4 CASTLE RIGHTS, 2*5 material counts
comps['piece']['input_neurons'] = 164 #
comps['square']['input_neurons'] = 128 #two board maps of 64 squares
comps['pawn']['input_neurons'] = 16 #8 files, 2 colors

layer1nodes = 50

comp_order = ["global", "piece", "square", "pawn"]

for comp in comps:
  input_name = "input_"+comp
  model.add_input(name = input_name, input_shape = (comps[comp]['input_neurons'],))
  model.add_node(Dense(comps[comp]['output_neurons'], activation = "relu"), name = comp, input = input_name)

num_features = sum([comps[comp]["input_neurons"] for comp in comps])
total_inputs = sum([comps[comp]["output_neurons"] for comp in comps])
SCORE_SCALE = 50.

def make_training_dict(x, y):
    training_dict = {}
    ind = 0
    for comp in comp_order:
      inc = comps[comp]["input_neurons"]
      training_dict["input_"+comp] = x[:,ind:ind+inc]
      ind += inc

    training_dict["out"] = y / SCORE_SCALE
    return training_dict

model.add_node(Dense(layer1nodes, activation = "relu"), name = "layer1", inputs= comp_order)
model.add_node(Dense(1, activation = "tanh"), name = "outlayer", input = "layer1")
model.add_output(name= "out", input = "outlayer")
model.compile(optimizer = "adagrad", loss = {"out":'mse'})

def save_as_npz(outfile):
    args = {}
    for name in model.nodes:
      node = model.nodes[name]
      args[name+"_weights"], args[name+"_biases"] = node.get_weights()[0], node.get_weights()[1]
    np.savez(outfile, **args)
