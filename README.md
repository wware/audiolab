# Burns organ as software on a Teensy

You want to generate blits of sound, basically doing what sndblit is
doing. You do the blits before you need them so that you can put sound
samples in a queue and pull them out at a steady 44.1 kHz rate.

So it probably makes sense to fool with sndblit just to see how to drive
it because it will functionally be the same back end.


