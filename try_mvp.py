# Minimum viable pipeline

from track import Track
from noises import Beep
from output import play_wave

t = Track()                   # defaults to creating a named temp directory
t.record(0, Beep(4, 800))     # one-second beep at 300 Hz, starting at zero seconds
t.record(1, Beep(3, 1000))
t.record(2, Beep(2, 1200))
t.record(3, Beep(1, 1600))
print t

t.write_wave('foo.wav')
play_wave('foo.wav')
