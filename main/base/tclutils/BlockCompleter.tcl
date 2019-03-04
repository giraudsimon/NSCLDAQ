#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
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

package provide BlockCompleter 1.0
package require snit


## @brief A utility for keeping track of when delimiters are balanced
#
# When reading lines of tcl source code from a file, it is likely that
# on any given line, there might be mismatched braces. If these lines are 
# then dealt with on an individual basis, it is possible that the application 
# will have errors resulting from mismatched braces. This snit::type provides a
# simple mechanism that allows the user to add bits of a complete statement
# and ask whether or not the built-up text has matching delimiters. The 
# class is not limited to the standard situation such as {}, (), or []. It can
# actually keep track are any "left" delimiter and any "right" delimiter. In
# other words, you could keep track of whenever there are an equal number of 
# "a" and "b" characters in a text. The limitation here is that the left and
# right delimiters must be unique. For example, the method isComplete would
# never return true if the user were trying to keep track of parentheses. (That
# is except for when no text has been added and both left and right delimiter
# counts are 0). 
#
# The algorithm is quite basic and simply counts the occurence of the delimiters
# in every new piece of text added. When count for the left and right delimiters 
# are not equal, the block is considered incomplete. Otherwise, it is complete.
#
snit::type BlockCompleter {

  option -left -default "{"   ;# left delimiter
  option -right -default "}"  ;# right delimiter

  variable _text 
  variable _nLeftDelimiters 
  variable _nRightDelimiters 

  ## @brief  Parse arguments and set variables to initial state
  #
  #
  constructor {args} {
    $self configurelist $args

    set _text {} ;# set to the empty string
    set _nLeftDelimiters 0
    set _nRightDelimiters 0
  }

  ## @brief Retrieve the text that is being built up 
  #
  # @returns the text
  method getText {} {
    return $_text
  }

  ## @brief Add new text
  #
  # The characters in the text being added are traversed and the delimiter
  # counts are adjusted to reflect the new state.
  #
  # @param text   the text to append to the end of the current text
  #
  method appendText {text} {
    $self updateDelimiterCounts $text
    append _text $text
  }

  ## @brief Check for balanced delimiters
  #
  # @returns boolean
  # @retval 0 - left delimiter and right delimiter counts do not match
  # @retval 1 - otherwise
  #
  method isComplete {} {
    return [info complete $_text]
  }

  ## @brief Traverse string and increment delimiter counts
  #
  # @brief text   the text to the parse
  method updateDelimiterCounts {text} {

    # traverse from element 0 to the last element
    set length [string length $text]
    set index 0

    while {$index < $length} {

      # retrieve current element
      set char [string index $text $index]

      # check if current element matched either delimiter
      if {$char eq [$self cget -left]} {
        incr _nLeftDelimiters
      } elseif {$char eq [$self cget -right]} {
        incr _nRightDelimiters
      } ;# otherwise do nothing

      incr index ;# move to the next element
    }
  }

  ## @brief Query current delimiter counts
  #
  # @returns list of counts. 
  #          Element 0 = left delim. count, Element 1 = right delim. count
  method getDelimiterCounts {} {
    return [list $_nLeftDelimiters $_nRightDelimiters]
  }


  ## @brief Reset the internal state 
  # 
  # The counters are reset to 0 and the text string maintained by this is
  # cleared.
  #
  method Reset {} {
    set _text {}
    set _nLeftDelimiters 0
    set _nRightDelimiters 0
  }
}
