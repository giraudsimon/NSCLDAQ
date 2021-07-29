#!/bin/bash

##
#  Shell script to do an export of a specific tag of the libtcl
#  Usage:
#    tcl++incorp.sh tag 
#
#  The export is done to the libtcl directory.
#
#
#  The idea is that libtcl/libexception could be or not be centrally
#  installed, but if not, built from the export directory.
#
# Last used with libtclplus-v4.1-006  - that's the minimum required
#  for 12.0  and newer, due to the movement of the Tcl++ datatypes
#  into the TCLPLUS namespace. Note that 4.0-000 won't work because of
# an error in TclObjectProcessor::getName that's fixed in 4.0-001
# that the logbook Tcl wrapping rely on.



##
# Note the repository specified below only is accessible
# within NSCL/FRIB.  To access externally, define the
# baseURL to be git://git.code.sf.net/p/nscldaq/libtclplus
#

baseURL="https://git.nscl.msu.edu/daqdev/libtclplus.git"

tag="$1"

rm -rf libtcl 

git clone $baseURL libtcl
(cd libtcl; git checkout tags/$1)


(cd libtcl; autoreconf -i)
