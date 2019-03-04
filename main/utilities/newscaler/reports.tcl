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
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

package provide scalerReport 1.0
package require report
package require struct::matrix
package require csv
package require scalerUtil

##
# @file reports.tcl
# @brief Write text files with reports at the end of run.
# @author Ron Fox <fox@nscl.msu.edu>
#


# Report style defs:

::report::defstyle simpletable {} {
    data	set [split "[string repeat "| "   [columns]]|"]
    top	set [split "[string repeat "+ - " [columns]]+"]
    bottom	set [top get]
    top	enable
    bottom	enable
}
::report::defstyle captionedtable {{n 1}} {
    simpletable
    topdata   set [data get]
    topcapsep set [top get]
    topcapsep enable
    tcaption $n        
}

##
# _formatElapsedTime
#   Turns a number of seconds into an elapsed time of the form>:
#     days hours:min:sec
#
# @param secs  - the seconds to formta
# @return string - The formatted elapsed time.
#
proc _formatElapsedTime secs {
    return [formatElapsedTime $secs]
}

##
# humanReport
#    Writes a human readable report to file.
#
# @param fd         - file descriptor to write to.
# @param started    - Time at which the run started ([clock seconds]).
# @param eor        - end of run dictionary.
# @param channels   - Channel test name-map that holds the scaler channels.
#
# @note the struct::matrix package is used to build up a matrix
#       consisting of the channel names, total counts, average rates and stddev
#       of rates.  This is then fed into the report package to create the body
#       of the output.  Note that the header is manually created.
# @note the scalers are reported in dictionary order (that is case blind alpha).
#
proc humanReport {fd started eor channels} {
    
    # Make and fill in the matrix:
    
    set scalerData [struct::matrix]
    $scalerData add columns 4
    $scalerData insert row end [list Name Total {Average Rate} {Rate std-dev}]
    set nameOrder [lsort -dictionary [$channels list]]
    foreach name $nameOrder {
        set channel [$channels get $name]
        
        set mean [format %0.2f [$channel mean]]
        set stddev [format %0.2f [$channel stddev]]
        
        set row [list $name [$channel total] $mean $stddev]
        $scalerData insert row end $row
    }
    
    # Create the report:
    
    
    ::report::report scalerReport 4 style captionedtable 1
    set report [scalerReport printmatrix $scalerData]
    
    # Clean up the bits and pieces we created:
    
    scalerReport destroy
    $scalerData  destroy
    
    # Now fill the output file:
    
    puts $fd "Run     :  [dict get $eor run]"
    puts $fd "Title   :  [dict get $eor title]"
    puts $fd "Started :  [clock format $started]"
    puts $fd "Ended   :  [clock format [dict get $eor realtime]]"
    
    set elapsed [dict get $eor timeoffset]
    set elapsed [_formatElapsedTime  $elapsed]
    puts $fd "Elapsed : $elapsed"
    puts $fd ""
    puts $fd $report
    
}

##
# computerReport
#   Writes a CSV (computer readable) file report.  The first line is:
# \verbatim
#     run number, title, start time, end time, elapsed time.
# \endverbatim
#  Subsequent lines are:
# \verbatim
#    scaler name, total counts, average rate, stddev of rate
# \endverbatim
#
# @note unlike the humanreadable reports, no rounding/truncation
#       is performed.
#
# @param fd         - file descriptor to write to.
# @param started    - Time at which the run started ([clock seconds]).
# @param eor        - end of run dictionary.
# @param channels   - Channel test name-map that holds the scaler channels.
#
proc computerReport {fd started eor channels} {
    
    # Header line:
    
    set run   [dict get $eor run]
    set title [dict get $eor title]
    set start [clock format $started]
    set end   [clock format [dict get $eor realtime]]
    set elapsed [dict get $eor timeoffset]
    
    puts $fd [csv::join [list $run $title $start $end $elapsed]]
    
    set nameOrder [lsort -dictionary [$channels list]]
    
    foreach name $nameOrder {
        set channel [$channels get $name]
        
        set mean [$channel mean]
        set stddev [$channel stddev]
        
        puts $fd [csv::join [list $name [$channel total] $mean $stddev]]
        
    }
    
}