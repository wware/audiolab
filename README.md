# Updating the sndblit concept

We can make global assumptions about fixed-point characteristics, time increment,
and T (half the duration of a blip) and those can go in a header file. Generally
speaking we will have dt = 1./44100, and T = N \* dt, with sample buffers of length N.
This is all coded in C++ with a Swig wrapper. When we run it on the Teensy we call
it as a C++ thing. On a laptop we can invoke it all with Python.

I want this to become C++ code that can run on Mac or Linux or Teensy. Let's have
a C-like function like this. When called, it adds the sinusoid to whatever is already
in the buffer.

    int sinusoid(int32_t **ptr, double freq, double phase,
                 double ampl1, double ampl2);
    ptr is a pointer to a sample buffer of known size
    phase is the phase at the beginning of the sample buffer
    ampl1 is amplitude at beginning
    ampl2 is amplitude at end of sample buffer
    return value is an error code of some sort, 0 if ok

https://docs.python.org/2.7/library/ctypes.html#arrays
https://docs.python.org/2.7/library/ctypes.html#pointers

    from ctypes import c_int
    N = 2048
    bufferType = c_int * N
    a = bufferType()
    r = sinusoid(a, ....)
    # do stuff, and then eventually garbage-collect a

# Burns organ as software on a Teensy

You want to generate blits of sound, basically doing what sndblit is
doing. You do the blits before you need them so that you can put sound
samples in a queue and pull them out at a steady 44.1 kHz rate.

So it probably makes sense to fool with sndblit just to see how to drive
it because it will functionally be the same back end.

There is now an option in sndblit where you can view the data as text
and it can be sent (with some text processing) to gnuplot.

    $ make sndblit
    $ ./sndblit -v -L L.data -R R.data < plugh
    $ less L.data R.data

