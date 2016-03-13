from network_arch import *
import numpy as np

model.load_weights("train_out_3_scaled2.hd5")
x = np.zeros((2,sum([comps[comp]["input_neurons"] for comp in comps])))
x[0,0] = 1

training_dict = {}
ind = 0
for comp in comp_order:
  inc = comps[comp]["input_neurons"]
  training_dict["input_"+comp] = x[:,ind:ind+inc]
  ind += inc

# print model.nodes["out"].get_input()

print model.predict(training_dict)
