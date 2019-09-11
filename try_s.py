#!/usr/bin/env python

# from ctypes import c_int32, pointer, POINTER
from foo import BUFSIZE, sinusoid

"""
extern int sinusoid(int32_t *ptr, double freq, double phase,
                    double ampl1, double ampl2);
"""

# bufferType = c_int32 * BUFSIZE
# a = bufferType()

import array

a = array.array('L', BUFSIZE * [0])
# p = POINTER(c_int32)(a)
# print p
r = sinusoid(a, 100.0, 0.0, 1.0, 1.0)
