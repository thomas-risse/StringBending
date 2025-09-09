# Plot data from output.txt
import numpy as np
import matplotlib.pyplot as plt
import sys
import os

# dir = "/Users/risse/work/StringBending/build"

# data = np.loadtxt(os.path.join(dir, 'output.txt'))
data = np.loadtxt('output.txt')
t = np.arange(len(data)) / 44100
# plt.plot(t, 20 * np.log10(np.abs(data) / np.max(np.abs(data))))
plt.plot(t, data)
plt.title('Output Trace')
plt.xlabel('Time (s)')
plt.ylabel('First modal coefficient')
plt.grid()
plt.tight_layout()
plt.show()

