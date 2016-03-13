from keras.models import Sequential
from keras.layers.core import Dense, Dropout, Activation
from keras.optimizers import SGD
import numpy as np

model = Sequential()
model.add(Dense(1, init = 'uniform', input_dim = 4))
# model.add(Dense(1, init = 'uniform'))
model.add(Activation('relu'))
model.compile(optimizer = 'sgd', loss = 'mse')
# f = lambda x: np.sum(x)
#
# data = np.random.random((1000,4))
# labels = np.array([f(el) for el in data])
#
# model.fit(data, labels, nb_epoch = 4, batch_size = 8, verbose = 1)
#
# tdata = np.random.random((1000,4))
# labels = np.array([f(el) for el in tdata])
# print model.test_on_batch(tdata, labels)
