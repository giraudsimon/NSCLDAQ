##
# Makefile for writerings
#
#   Assumes an daqsetup.bash has been sourced so that DAQROOT etc.
#   are defined:


all: writerings

writerings: writerings.cpp
	g++ -o writerings writerings.cpp 		\
	-I$(DAQROOT)/include -L$(DAQLIB)	\
	-ldataformat -ldaqio -lException -Wl,-rpath=$(DAQLIB) -std=c++11


clean:
	rm -f writerings
