#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2015.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321


package provide TclSourceFilter 1.0

package require BlockCompleter

## @brief Extracts specific patterns from a script
#
# This snit::type can be loaded with a list of regular expressions that are used
# to extract full blocks from a tcl script. This takes the original script,
# breaks it into complete blocks, and then checks those blocks against the
# specified patterns. 
#
# Any pattern provided is used in conjunction with the regexp command.
snit::type TclSourceFilter {

  variable _validPatterns ;#!< list of regular expressions


  ## @brief Construct and set up the state
  #
  # @param presenter    an MCFD16ControlPanel instance
  # @param args         option-value pairs
  #
  constructor {args} {
    set _validPatterns [list]
  }
  
  #--- Public interface


  ## @brief Pass list of regular expressions
  #
  # @param patterns   list of regular expressions
  method SetValidPatterns {patterns} {
    set _validPatterns $patterns
  }

  ## @brief Entry point for processing
  #
  # @param  path    the path to a file containing state
  # @return list - of strings that make up the matching patterns.
  #
  method Filter {script} {
    # split the file into complete chunks that can be passed to eval
    set rawLines [$self Tokenize $script]

    # find the lines we can safely execute
    set executableLines [$self FindPatternMatches $rawLines]

    return $executableLines
  }

  #---- Utility methods

  ## @brief Split the file into fully executable chunks
  #
  # This uses the BlockCompleter snit::type to find all complete blocks (i.e. at
  # the end of a line the number of left and right curly braces are equal). The
  # list of complete blocks are returned.
  #
  # @param path   the path to the file to parse
  #
  # @returns  the list of complete blocks 
  method Tokenize {contents} {

    set blocks [list]
    BlockCompleter bc -left "{" -right "}"
 
    # split the complete contents into a list of lines
    set lines [split $contents "\n"]

    # work through the list until it is done.
    set index 0
    set nLines [llength $lines]
    while {$index < $nLines} {
      bc appendText [lindex $lines $index]
      incr index
      while {![bc isComplete] && ($index < $nLines)} {
        bc appendText "\n[lindex $lines $index]"
        incr index
      }
      lappend blocks [bc getText]
      bc Reset
    }

    bc destroy
    return $blocks
  }

  ## @brief The actual filter code
  #
  # Passed a list of complete tcl blocks, this then attempts to match each block
  # to the provided patterns. If the match is good (i.e. regexp returns 1) then
  # the block is appended to the list that is returned. 
  #
  # @param  blocks   the list of complete blocks
  #
  # @returns a list of lines that matched 
  method FindPatternMatches blocks {
    set validLines [list]
    foreach line $blocks {
      if {[$self IsPatternMatch $line]} {
        lappend validLines $line
      }
    }
    return $validLines
  }

  ## @brief Check whether line matches on of the provided expressions
  #
  # This does a linear search through the list of patterns and stops only when
  # it has found a positive match or has run out of regular expressions to try.
  #
  # @param line   line of code to check
  #
  # @returns boolean
  # @retval 0 - second element is not in list of valid calls
  # @retval 1 - otherwise
  method IsPatternMatch {line} {
    set found 0
    foreach patt $_validPatterns {
      set found [regexp $patt $line]
      if {$found} break
    }

    return $found
  }

} ;# end of TclSourceFilter
