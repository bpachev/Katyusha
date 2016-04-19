import sys
import numpy as np
import matplotlib.pyplot as plt

logfile=None
usage = "usage logfile"
if len(sys.argv) < 2:
  print usage
  sys.exit(1)
else:
  logfile = sys.argv[1]

l2_loss = []
l1_loss = []
with open(logfile, "r") as f:
  for line in f:
    a = line.strip().split()
    if len(a) < 3: continue
    if a[1] == "loss":
        if a[0] == "l1":
            l1_loss.append(float(a[2]))
        elif a[0] == "l2":
            l2_loss.append(float(a[2]))

plt.subplot(211)
plt.plot(np.array(l1_loss))
plt.title("L1 loss")
plt.subplot(212)
plt.plot(np.array(l2_loss))
plt.title("L2 loss")
plt.show()
