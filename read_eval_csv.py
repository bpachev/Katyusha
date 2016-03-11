import numpy as np
from sys import argv, exit

argc = len(argv)

usage = "evaluations features outfile"
if argc < 4:
 print usage
 exit()

e = open(argv[1], "r")
f = open(argv[2], "r")

curr_game = 0
max_positions = 380000
#max_positions = 10
features = 323
training_x = np.zeros((max_positions, features))
training_y = np.zeros(max_positions)


for game in xrange(max_positions): 
  evals = np.array(map(float,e.readline().strip().split(",")))
  npositions = evals.size
  pos_number = np.random.randint(0,npositions)
#  print "Choosing Position number %d" % pos_number
  training_y[game] = evals[pos_number]
  for j in xrange(evals.size+1):
   if j == pos_number:
     line = f.readline()
     training_x[game, :] = np.array(map(float,line.strip().split(",")))
   else:
     f.readline()

np.savez(argv[3], training_x = training_x, training_y = training_y) 



