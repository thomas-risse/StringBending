import numpy as np
from scipy.fftpack import fft, dct
import timeit
import matplotlib.pyplot as plt

"""
Crude test of naive dense matrix vector mulitplication vs fft version for matrices
of sines.
"""

#%%
Ntot = 10
Nmodes = np.array([2**(i+1)+1 for i in range(Ntot)])

timeNaive = np.zeros((Ntot))
timeFFT = np.zeros((Ntot))

for i, N in enumerate(Nmodes):
  print(i)
  # Build the matrix
  Mat = np.zeros((N, N))

  coords = (2 * np.arange(N) +1 )/ (2 * N)
  for j in range(N):
    Mat[j, :] = 2 * np.cos((j) * coords * 2 * np.pi)

  # Random vector
  vec = np.random.random(N)
  vec = np.ones(N)

  def naiveMult():
    out = Mat @ vec 
    return out

  def FFTMult():
    out = dct(vec)
    return out

  # print(Mat)
  # print(naiveMult())
  # print(FFTMult())
  timeNaive[i] = (timeit.timeit("naiveMult()", globals=globals()))
  timeFFT[i] = (timeit.timeit("FFTMult()", globals=globals()))


plt.figure()
plt.plot(Nmodes, timeNaive, label = "naive")
plt.plot(Nmodes, timeFFT, label = "FFT")

plt.xlabel("Number of modes")
plt.ylabel("Elapsed time (us)")
plt.legend()
plt.grid()
plt.savefig("FFTSpeed2.pdf")
plt.show(block=True)
