#UNAME := $(shell uname)
## dotted python version (2.3, 2.4)
#PYDVER := $(shell python -c "import sys; print sys.version[:3]")
## un-dotted python version (23, 24)
#PYVER := $(shell python -c "import sys; print sys.version[0]+sys.version[2]")
#
#UNAME_A=$$(uname -a)
#CC=gcc
#PYREXTARGET=atombase.so
#STDC99=-std=c99
#ifeq ($(strip $(UNAME)),Darwin)
##---------------------------------------- Mac
#PYBASE=/usr/local/Cellar/python/2.7.8_1/Frameworks/Python.framework/Versions/2.7
#CFLAGS=-I/System/Library/Frameworks/Python.framework/Versions/$(PYDVER)/lib/python$(PYDVER)/config \
#   -I/System/Library/Frameworks/Python.framework/Versions/$(PYDVER)/include/python$(PYDVER)/ \
#   -DMACOSX -I/System/Library/Frameworks/OpenGL.framework/Headers
#LDFLAGS=-Wl,-F. -framework Python
#LDFLAGS+=-L/usr/lib -lm
### LDFLAGS+=-L/usr/X11R6/lib -lGL
#LDFLAGS+=-framework OpenGL
#LDSHARED=gcc -bundle
#
## See if we can add Universal build flags
#UNAME_M=$(shell uname -m)
#ifeq ($(UNAME_M),i386)
#CFLAGS+=-arch i386 -arch ppc
#LDFLAGS+=-arch i386 -arch ppc
#endif

# dotted python version (2.3, 2.4)
PYDVER := $(shell python -c "import sys; print sys.version[:3]")
# un-dotted python version (23, 24)
PYVER := $(shell python -c "import sys; print sys.version[0]+sys.version[2]")

PYBASE=/usr/local/Cellar/python/2.7.8_1/Frameworks/Python.framework/Versions/$(PYDVER)
INCL=-I$(PYBASE)/include/python$(PYDVER)
PYLIB=$(PYBASE)/lib/python$(PYDVER)/config/libpython$(PYDVER).a
PYTHON=$(PYBASE)/bin/python

swig: foo_wrap.cxx foo.o
	g++ -c -fpic $(INCL) foo_wrap.cxx
	g++ -dynamiclib foo_wrap.o foo.o $(PYLIB) -o _foo.so

foo: foo.o
	g++ -o foo foo.o

foo.o: foo.cpp
	g++ -c -o foo.o foo.cpp

clean:
	rm -f *.pyc *.o *.so foo.py foo_wrap.cxx foo F G H

foo_wrap.cxx: foo.h foo.i
	swig -python -c++ foo.i

run:
	$(PYTHON)
