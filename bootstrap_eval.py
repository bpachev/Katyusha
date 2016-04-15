from keras.models import Sequential, Graph
from keras.layers.core import Dense, Dropout, Activation
from keras.optimizers import SGD
from network_arch import *
import numpy as np
from sys import argv, exit

if len(argv) < 3:
  print "usage trainingdatafile outfile"
  exit()


data = np.load(argv[1])
x,y = data['training_x'], data['training_y']


training_dict = make_training_dict(x, y)
model.fit(training_dict, validation_split = .25, verbose = 1, nb_epoch = 50)

model.save_weights(argv[2], overwrite=True)
