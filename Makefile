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
CFLAGS=$$(python2-config --cflags) -g -fPIC
LDFLAGS=$$(python2-config --ldflags) -g -shared
endif

###############################

CC = gcc
# CFLAGS = -O3 -Wall
# CFLAGS = -g -Wall

###############################

all: xx.so

xx.so: xxmodule.o
	gcc $(LDFLAGS) xxmodule.o -lm -lpython$(PYDVER) -o xx.so

xxmodule.o: xxmodule.c
	gcc -c $(CFLAGS) -o xxmodule.o xxmodule.c

clean:
	rm -f *.pyc *.o *.so

run: xx.so
	$(PYTHON)
