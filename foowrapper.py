import ctypes

lib = ctypes.CDLL('./foo.so')
a = ctypes.c_int * lib._BUFSIZE
print lib.sinusoid(3, 100, 1, 4, 5)
