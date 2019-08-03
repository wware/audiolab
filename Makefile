###############################

UNAME := $(shell uname)
# dotted python version (2.x)
PYDVER := $(shell python -c "import sys; print sys.version[:3]")
# un-dotted python version (23, 24)
PYVER := $(shell python -c "import sys; print sys.version[0]+sys.version[2]")

###############################

ifeq ($(strip $(UNAME)),Darwin)
# OSX
PYBASE=/usr/local/Cellar/python/2.7.8_1/Frameworks/Python.framework/Versions/$(PYDVER)
INCL=-I$(PYBASE)/include/python$(PYDVER)
PYTHON=$(PYBASE)/bin/python
FPIC=-fpic
SHARED=-dynamiclib
else
# Linux
PYBASE=$$(python2-config --prefix)
INCL=-I$(PYBASE)/include/python2.7
PYTHON=$(PYBASE)/bin/python
FPIC=-fPIC
SHARED=--shared
CFLAGS=$$(python2-config --cflags) -g
LDFLAGS=$$(python2-config --ldflags) -g
endif

###############################

CC = gcc
# CFLAGS = -O3 -Wall
CFLAGS = -g -Wall

###############################

all: _foo.so sndblit

_foo.so: foo_wrap.cxx foo.o
	g++ -c $(CFLAGS) $(FPIC) $(INCL) foo_wrap.cxx
	g++ $(LDFLAGS) foo_wrap.o foo.o -lpython$(PYDVER) -o _foo.so

F: foo
	./foo > F

foo: foo.o
	g++ -o foo foo.o

foo.o: foo.cpp
	g++ -c $(CFLAGS) -o foo.o foo.cpp

clean:
	rm -f *.pyc *.o *.so foo.py foo_wrap.cxx foo F G H sndblit

foo_wrap.cxx: foo.h foo.i
	swig -python -c++ foo.i

run: _foo.so
	$(PYTHON)

sndblit: sndblit.c
	$(CC) $(CFLAGS) -o sndblit sndblit.c -lm

install: sndblit
	sudo cp sndblit /usr/local/bin
	sudo cp sndblit.1 /usr/share/man/man1
