import math
from common import SAMPLE_RATE

DT = 1. / SAMPLE_RATE



class Beep(object):
    def __init__(self, duration, freq):
        self._duration = duration
        self._freq = freq
        self._t = 0.

    def done(self):
        return self._t >= self._duration

    def next_sample(self):
        if self.done():
            sample = 0.
        else:
            sample = math.sin(2 * math.pi * self._freq * self._t)
        self._t += DT
        return sample
