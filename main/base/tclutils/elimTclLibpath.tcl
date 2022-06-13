#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Giordano Cerriza
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


##
# @file elimTclLibpath.tcl
# @brief Remove directories in TCLLIBPATH from package search path.
# @author Ron Fox <fox@nscl.msu.edu>
#


package provide removetcllibpath 1.0


##
# The pointof this package is that users sometimes have their own TCLLIBPATH
# environment variable defined.  This can lead to cases where we have a mix
# of packages loaded from the correct and incorrect NSCLDAQ versions. Hard as
# hell to diagnose.  This package removes TCLLIBPATH search directories from
# the Tcl package search path.  Requiring the package is sufficient to
# remove those directories as it defines and then calls ExciseTclLibpath::removeTclLibPath.
#
    
namespace eval ExciseTclLibpath {
 proc removeTclLibPath {} {
     if {[array names ::env TCLLIBPATH] ne ""} {
         set badDirs $::env(TCLLIBPATH)
         set path    $::auto_path
         foreach dir $badDirs {
             set i [lsearch -exact $path $dir]
             set path [lreplace $path $i $i]
         }
         set ::auto_path $path
     }
  }
}
::ExciseTclLibpath::removeTclLibPath
