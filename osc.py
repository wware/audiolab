import unittest
from math import pi, sin, fmod

TWOPI = 2 * pi
_A = 1. / pi
_B = 2. / pi


class Oscillator(object):
    def __init__(self, f):
        self._f = f

    def phase(self, t):
        ph = TWOPI * self._f * t
        return fmod(ph, TWOPI)

    def pwm(self, t, p):
        return 1. if self.phase(t) > p else -1.

    def sine(self, t):
        return sin(self.phase(t))

    def ramp(self, t):
        return _A * self.phase(t) - 1.

    def triangle(self, t):
        p = self.phase(t)
        if p < pi:
            return _B * p - 1.
        else:
            return -_B * p + 3.


class TestOsciallator(unittest.TestCase):
    def approx(self, x, y):
        if abs(x - y) > 1.e-6:
            raise AssertionError("{0} is not near {1}".format(x, y))

    def test_sine(self):
        osc = Oscillator(1)
        self.approx(osc.sine(0), 0.)
        self.approx(osc.sine(0.25), 1.)
        self.approx(osc.sine(0.5), 0.)
        self.approx(osc.sine(0.75), -1.)

    def test_ramp(self):
        osc = Oscillator(1)
        self.approx(osc.ramp(0), -1.)
        self.approx(osc.ramp(0.25), -.5)
        self.approx(osc.ramp(0.5), 0.)
        self.approx(osc.ramp(0.75), .5)

    def test_triangle(self):
        osc = Oscillator(1)
        self.approx(osc.triangle(0), -1.)
        self.approx(osc.triangle(0.25), 0.)
        self.approx(osc.triangle(0.5), 1.)
        self.approx(osc.triangle(0.75), 0.)


if __name__ == '__main__':
    unittest.main()
