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
# @file lg_noteutilities.tcl
# @brief Provide utilities for note composition.
    

# @author Ron Fox <fox@nscl.msu.edu>
#
package provide lg_noteutilities 1.0
package require logbookadmin

#------------------------------------------------------------------------------
# Private entries.
#

##
# _getLinkInfo
#   Given a string and the offset of the ![ in  an image link finds:
#   - The filename in the image link (stuff inside of ()), and
#   - The character offset in the string just past the image link.
#
# @param text  - The full text string.
# @param index - Location of ![
# @return (int string) - list of end offset and filename.
#
proc _getLinkInfo {text index} {
    
    # Locate the ( and );  Failure to find either of these is an error.

    set pStart [string first "(" $text $index]
    set pEnd   [string first ")" $text $index]
    
    # Neither are allowed to be -1:
    
    if {($pStart == -1) || ($pEnd == -1)} {
        error "Invalid image link in text near '[string range $text index index+20]...'"
    }
    set filename [string range $text $pStart+1 $pStart-1]
    incr pEnd
    return [list $filename $pEnd]
}

#------------------------------------------------------------------------------
# Public entries

##
# validateImageFiles
#  Given a list of pairs; the first element of each pair an offset, the second
#  element of each pair a file path, returns a list of the filenames that cannot
#  be opened and read.  This list, for a well formed markdown file will be empty.
#
# @param images - the image list.  
# @return list of strings - possibly empty list of unreadable image files.
# @note no effort is made to determine if these files are sensible image files.
#
proc validateImageFiles {images} {
    set result [list]
    foreach image $images {
        set filename [lindex $image 1]
        if {![file readable $filename]} {
            lappend result $filename
        }
    }
    
    return $result
}



##
# makeNoteImageList
#    Given a soup of note text, returns a list (possibly empty) of the image
#    link locations and file paths.
# @param text
# @return list of pairs - first element of each pair is a character offset. The
#         second the filename associated with the image link at that offset.
#
proc makeNoteImageList {text} {
    set size [string length $text]
    set index 0
    set result [list]
    while {$index < $size} {
        set start [string first "!\[" $text $index]
        if {$start >= 0} {
            
            # Locate and pull out the filename and end of the link:
            
            set linkInfo [_getLinkInfo $text $start]
            set name [lindex $linkInfo 0]
            set end  [lindex $linkInfo 1]
            
            lappend result [list $start $name]           
            
            set index $end
        } else {
            set index $size
        }
    }
    return $result
}