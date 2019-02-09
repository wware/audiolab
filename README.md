# AudioLab

Software equivalent of a lot of pieces of audio equipment. Make all this stuff
easily scriptable.

## Audio information format

Tracks are monophonic signed-integer 16-bit samples at 44.1 kHz. Each file is
2K bytes, or 1024 samples, representing about 25-ish milliseconds. Filenames are
sequential eight-character lowercase names running from "aaaaaaaa" to "zzzzzzzz".
Tracks are directories full of files. Partially-filled files are padded with zeros.

Write a C library for working with this arrangement and give it a Python
binding. Where it's helpful, throw in metadata files (for instance, maybe
identify peaks for all sample files).

## Things to be able to do

Be able to play sounds backwards, or at different speeds (Alvin the chipmunk).

## Components

* Output sink that produces a stereo audio file in a standard format.
* Mixers, with a choice of options to prevent clipping: linear-1/N, A-law, mu-law.
* Delays of different sorts. Be able to build multiple-delay and echo things.
* Equalizers, both graphic and parametric.
* A few simple sources for testing, like importing from standard audio files, or simple synths.
* Other audio components as they come to mind.

## Process vibes

This looks like a good place to use TDD. Testing might not be automated, it would
probably be more like "try this and listen to the result and see if it sounds right".
Automation could come later.

I want to tinker with using "mock" as a design tool. Not sure exactly what this would
entail, or whether it would be as useful as I imagine it might be, but the general idea
is to start with mocks for things, and then fill in behaviors. Starting with mocks might
facilitate the creation of tests.

Plan on a minimum viable pipeline (source -> track -> audio file -> listen) and
get that working first, then build stuff up from there, including echo/delays and
mixers and all that.

Might as well use TDD as an excuse to work from the UI inward. So write the script
you want for the MVP, and then put the stuff under it to make it work.
