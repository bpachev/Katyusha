from keras.models import Sequential, Graph
from keras.layers.core import Dense, Dropout, Activation
from keras.optimizers import SGD
import numpy as np
from sys import argv, exit

if len(argv) < 3:
  print "usage trainingdatafile outfile"
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

for comp in comps:
  input_name = "input_"+comp
  model.add_input(name = input_name, input_shape = (comps[comp]['input_neurons'],))
  model.add_node(Dense(comps[comp]['output_neurons'], activation = "relu"), name = comp, input = input_name)

total_inputs = sum([comps[comp]["output_neurons"] for comp in comps])

model.add_node(Dense(layer1nodes, activation = "relu"), name = "layer1", inputs= comps.keys())
model.add_node(Dense(1, activation = "tanh"), name = "outlayer", input = "layer1")
model.add_output(name= "out", input = "outlayer")
model.compile(optimizer = "sgd", loss = {"out":'mse'})

data = np.load(argv[1])
x,y = data['training_x'], data['training_y']


training_dict = {}
comp_order = ["global", "piece", "square", "pawn"]
ind = 0
for comp in comp_order:
  inc = comps[comp]["input_neurons"]
  training_dict["input_"+comp] = x[:,ind:ind+inc]
  ind += inc

SCORE_SCALE = 50.
training_dict["out"] = y / SCORE_SCALE 
model.fit(training_dict, validation_split = .25, verbose = 1, nb_epoch = 75)
model.save_weights(argv[2])

