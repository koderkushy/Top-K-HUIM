import matplotlib.pyplot as plt
import numpy as np


a = [1, 3, 2, 5, 4]
b = [3, 4, 1, 2, 5]
c = [10, 15, 20, 25, 30]

fig, ax = plt.subplots()

ax.plot(a, label = 'l1')
ax.plot(b, label = 'l2')
ax.legend(loc = 'upper left')

plt.show()
