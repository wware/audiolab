import array
import string
import tempfile

import mock
import pdb

from common import SAMPLE_RATE, TODO

BLOCKBITS = 10
BLOCKSIZE = (1 << BLOCKBITS)     # 1024 samples in each block

SCALEBITS = 12
SCALE = 1 << SCALEBITS

BITS_PER_SAMPLE = 16
BYTES_PER_SAMPLE = BITS_PER_SAMPLE / 8
NUM_CHANNELS = 1

# a = array.array('h')     # 16-bit signed ints, native endian


class Cursor(object):
    def __init__(self, blockindex=0, index=None):
        if index is None:
            self.position(blockindex)
        else:
            self._blockindex = blockindex
            self._index = index

    @classmethod
    def at_time(self, t):
        n = int(1. * SAMPLE_RATE * t)
        return Cursor(n)

    def position(self, settable=None):
        if settable is not None:
            self._blockindex = settable >> BLOCKBITS
            self._index = settable & (BLOCKSIZE - 1)
        return (self._blockindex << BLOCKBITS) + self._index

    def __add__(self, delta):
        assert isinstance(delta, (int, long))
        self.position(self.position() + delta)
        return self

    def filename(self):
        r = ""
        p = self._blockindex
        for i in range(8):
            n, p = p % 26, p / 26
            r = string.lowercase[n] + r
        return r


class Track(object):
    def __init__(self, path=None):
        if path is None:
            path = tempfile.mkdtemp()
        self._path = path
        self._buffers = []
        self._cursor = Cursor()

    def __repr__(self):
        return "<Track@{0:x} {1} seconds>".format(id(self), self.num_seconds())

    def num_seconds(self):
        return (1. * BLOCKSIZE * len(self._buffers)) / SAMPLE_RATE

    def num_bytes(self):
        return BYTES_PER_SAMPLE * BLOCKSIZE * len(self._buffers)

    def __getslice__(self, i, j):
        raise TODO()

    def __setslice__(self, i, j, sequence):
        raise TODO()

    def __getitem__(self, i):
        if isinstance(i, (int, long)):
            c = Cursor(i)
        elif isinstance(i, Cursor):
            c = i
        else:
            raise TypeError((i, type(i)))
        while len(self._buffers) <= c._blockindex:
            self._add_buffer()
        b = self._buffers[c._blockindex]
        return b[c._index]

    def __setitem__(self, i, x):
        if isinstance(i, (int, long)):
            c = Cursor(i)
        elif isinstance(i, Cursor):
            c = i
        else:
            raise TypeError((i, type(i)))
        while len(self._buffers) <= c._blockindex:
            self._add_buffer()
        b = self._buffers[c._blockindex]
        b[c._index] += x

    def _add_buffer(self):
        self._buffers.append(array.array('h', BLOCKSIZE*[0]))

    def record(self, time, thing):
        c = Cursor.at_time(time)
        while not thing.done():
            sample = thing.next_sample()
            if isinstance(sample, float):
                sample = int(SCALE * sample)
            self[c] = sample
            c += 1

    def write_wave(self, f):
        # write a RIFF file
        with open(f, 'w') as outf:
            def bytes(n, x):
                # little endian
                for _ in range(n):
                    y, x = x & 255, x >> 8
                    outf.write(chr(y))
            # http://www.topherlee.com/software/pcm-tut-wavformat.html
            # https://en.wikipedia.org/wiki/Resource_Interchange_File_Format
            PCM = 1   # ????
            outf.write("RIFF")
            bytes(4, self.num_bytes() + 36)
            outf.write("WAVEfmt ")
            bytes(4, 16)    # length of format data so far
            bytes(2, PCM)
            bytes(2, NUM_CHANNELS)
            bytes(4, SAMPLE_RATE)
            bytes(4, SAMPLE_RATE * BYTES_PER_SAMPLE * NUM_CHANNELS)
            bytes(2, BYTES_PER_SAMPLE * NUM_CHANNELS)
            bytes(2, BITS_PER_SAMPLE)
            outf.write("data")
            bytes(4, self.num_bytes())
            for b in self._buffers:
                b.tofile(outf)


#### tests

import unittest


class CursorTest(unittest.TestCase):
    def test_1(self):
        lst = []
        for i in range(26**3):
            c = Cursor(i)
            lst.append(c.filename())
        L = lst[:]
        L.sort()
        self.assertEqual(L, lst)
        self.assertEqual(lst[-1], 'aaaaaaar')

if __name__ == '__main__':
    unittest.main()
