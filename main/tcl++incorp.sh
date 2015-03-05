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
#  
#  NOTE: As of September 19, 2013 the tab to use is
#       libtclplus-v2.0-001
# 

baseURL="https://svn.code.sf.net/p/nscldaq/code/tags"

tag="$1"

URL="$baseURL/$tag"

echo $URL

svn export $URL libtcl

(cd libtcl; autoreconf -i)
