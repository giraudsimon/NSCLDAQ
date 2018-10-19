##
# Makefile for readrings
#
#   Assumes an daqsetup.bash has been sourced so that DAQROOT etc.
#   are defined:


all: readrings

readrings: readrings.cpp
	g++ -o readrings readrings.cpp 		\
	-I$(DAQROOT)/include -L$(DAQLIB)	\
	-ldataformat -ldaqio -lException -Wl,-rpath=$(DAQLIB) -std=c++11


clean:
	rm -f readrings
