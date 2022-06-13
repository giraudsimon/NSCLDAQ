# actions.tcl
#
# A tcl package to handle the parsing of a basic messaging protocol
# to allow communication between devices in a pipeline and the ReadoutGUI.
#
# Things that need to be fixed. 
#   - Need to fix the leaking of the process.
#   - sometimes a newline is prepended to an output and that needs to be 
#     fixed.
#


package provide Actions 1.0
package require snit


## 
#  A TCL-side parser for reading messages out of a pipeline enabled
#  for reading. This aims to handle messages that are bookended with
#  newline characters and will not work properly if messages do not
#  follow that rule. Within that framework, this will identify 
#  packets that are formed by the C++ functions in the Actions namespace.
#  These have the form "DIRECTIVE size message", where DIRECTIVE is either
#  ERRMSG, LOGMSG, WRNMSG, TCLCMD, OUTPUT, or DBGMSG. Any other message 
#  that is not in this form will be treated as output.
#
#  This snit::type only handles the parsing stage of the message handling.
#  What is actually done with the messages after parsing is entirely up
#  to the user and definable via a callback bundle (-actionbundle). By 
#  default, a callback bundle is provided and used that will work with
#  the ReadoutGUI logging mechanism for formatting data.
snit::type Actions {
  
  variable accumulatedInput ""   ;#< message data received for parsing, split into a list by newlines 
  variable accumulatedOutMsg ""  ;#< message being formed as a packet to output
  variable incomplete 0          ;#< flag identifying if processing ended with an incomplete packet
  variable errors [dict create 0 " unable to parse directive"] 
  variable legalDirectives {ERRMSG LOGMSG WRNMSG TCLCMD OUTPUT DBGMSG} 
  variable directiveMap { ERRMSG 0 LOGMSG 1 WRNMSG 2 
                          TCLCMD 3 OUTPUT 4 DBGMSG 5} 

  option -actionbundle -default DefaultActions ;#< callback bundle
  option -exitok -default 0

  constructor {args} {
    $self configurelist $args
  }

  ## \brief Method for responding to readable events on pipes
  # 
  # This is really the entry point to this whole business. It 
  # deals with eof conditions properly. Usually it just delegates
  # to the handleReadable method
  #
  # \param fd   the file channel to read from
  method onReadable {fd} {

    if { [eof $fd] } {
      # unregister itself
      chan event $fd readable ""

      # we have reached an end of file
      #
      # convert to blocking to retrieve the exit status
      chan configure $fd -blocking 1

      if {[catch {close $fd} msg]} {
        if {[lindex $::errorCode 0] eq "CHILDSTATUS"} {
          puts stderr "Child process exited abnormally with status: $::errorCode"
          
        }

      }
      #puts stderr "Exiting with $options(-exitok)"

      if {!$options(-exitok)} {
        #  If this is not ok be noisy about this:
        tk_messageBox -type ok -icon error -title "Unexpected actionchild exit" \
          -message "Action handler detected child exited unexpectedly"
      }
    } else {
      $self handleReadable $fd 
    }
  }

  method getLine {} { return $accumulatedInput }
  method setLine {str} { set accumulatedInput $str }

  ## \brief Reads input from the channel and processes it
  #
  # \param fd   the channel to read from
  method handleReadable {fd} {

    # read what the channel has to give us, this does not terminate in 
    # a newline if there was none to begin with.
    set input [chan read $fd ]

    if {[catch {$self processInput $input} msg]} {
      puts "processInput failed with $input $::errorInfo"
      $self handleError "Error while parsing '$input'"
      $self handleError "Error: $msg"
      $self handleError "Resetting parser state"
      set accumulatedInput {}
      set accumulatedOutMsg {}
      set incomplete 0
    }
    #puts "{$msg}"
    return $msg
  }

  ## Adjust the number of lines we have to process
  #
  # We need to be careful about not processing the last line of the message
  # in case it did not end in a newline character. By not ending in a newline
  # character, that means we do not have a complete message and should
  # adjust the number of lines to process so that the last line is excluded.
  method computeNLinesToProcess {nLines strippedNewline firstLine} {
    if {(! $strippedNewline)} {
      #puts "did not strip newline"
      #puts "nLines=$nLines, firstLine=\"$firstLine\", isLegalDir=[$self isLegalDirective [$self extractFirstWord $firstLine]]"
      if {($nLines != 1) 
          || !([$self isLegalDirective [$self extractFirstWord $firstLine]]) 
          || ([string length $firstLine]<6)} {
        #puts "adjusting number of lines"
        incr nLines -1
      }
    }

    return $nLines
  }


  ## The logic for parsing the messages
  #
  # This has become embarrassingly complicated... but it is not reasonable to rewrite.
  # Parsing is not a trivial task.
  #
  # The idea here is that we are maintaining a list of lines that were separated by 
  # newline characters. Packets may be contained in a single line or may consist of multiple lines.
  # The packet message that is being formed is therefore part of the state of the 
  # snit::type and is appended to until it is determined complete. 
  #
  # Once a packet is completed, it is added to a list of results. You can imagine that the 
  # possibility exists to have multiple packets to deal with if we are not line buffering.
  # In that case, we keep adding complete packets to our list until all viable text has
  # been processed and then the packets are processed at the end.
  method processInput input {
    
    #puts "processing input: \"$input\""

    if {$incomplete} {
      #puts "last was incomplete and we start with : \"$accumulatedInput\""
      set input "[lindex $accumulatedInput end]$input"
      set accumulatedInput {}
    }

    set strippedNewline 0
    set result {}

    # pop a single newline character off of the end if it exists to avoid 
    # an empty line at the end caused by the split command
    if {[string index $input end] eq "\n"} {
      set input [string range $input 0 end-1]
      set strippedNewline 1
      #puts "stripped the newline!"
    }

    set accumulatedInput [concat $accumulatedInput [split $input "\n"]]
    #puts "accumulatedInput: \"$accumulatedInput\""

    # we need to iterate over all lines that are present at the start
    # so we will iterate over a copy of accumulatedInput because 
    # the accumulatedInput list gets manipulated as we go along
    set linesToProcess $accumulatedInput
    set nLines [llength $linesToProcess]
    if {$nLines == 0} {incr nLines}
    set firstLine [lindex $linesToProcess 0]

    # adjust the number of lines to process in case we believe the last line is incomplete
    set nLinesToProcess [$self computeNLinesToProcess $nLines $strippedNewline $firstLine]
    #puts "nLines: $nLines, nLinesToProcess: $nLinesToProcess"

    # process the lines of data
    for {set index 0} {$index<$nLinesToProcess} {incr index} {

      set line [lindex $linesToProcess $index]


      # correct for the removal of newlines during the split command
      if {($index == [expr $nLines-1]) && !$incomplete} {
        # if we are processing all of the original lines, then that implies the
        # message we are working with is considered a complete message
        append accumulatedOutMsg "$line"
      } else {
        # because we are here, we are in the middle of a message, and it is possible
        # that we had newlines in the middle of the message. Make sure to add back the newline character
        #that 
        append accumulatedOutMsg "$line\n"
      }

      # pop off the current line from accumulatedInput
      set accumulatedInput [lreplace $accumulatedInput 0 0]

      #puts "line: \"$line\", accumulatedOutMsg: \"$accumulatedOutMsg\", input: \"$accumulatedInput\", incomplete=$incomplete"

      # if the first word is a legal directive then we will handle the 
      # message as a packet, otherwise it is some other entity 
      set firstWord [$self extractFirstWord $accumulatedOutMsg]
      if {[$self isLegalDirective $firstWord]} {
        #puts "Found a directive"
        set parsedLine [$self buildPacket $accumulatedOutMsg]
        if {$parsedLine ne ""} {
          set incomplete 0
          lappend result $parsedLine
          set accumulatedOutMsg {}
        } ;# else move to next line and handle it
      } else {
        #puts "Found a non packet"
        set parsedLine [$self handleNonPacket $accumulatedOutMsg]
        if {$parsedLine ne {}} {
          lappend result $parsedLine
          if {[lindex $parsedLine 2] == [string length $accumulatedOutMsg]} {
            set accumulatedOutMsg {}
          } else {
            set accumulatedOutMsg [string range $accumulatedOutMsg [lindex $parsedLine 2] end]
          }
        }
      } 
    } ;# end of for loop


    # handle the messages!
    set incomplete [expr !$strippedNewline]
    #puts "result: \"$result\""
    set retval {}
    foreach msg $result {
      lappend retval [$self handleMessage $msg]
    }
    return $retval
  }

  method extractFirstWord {sentence} {
    return [string range $sentence 0 5]
  }

  ## Parse msg that we have identified to begin with a directive
  #
  # The first word was detected as a legal directive so 
  # we expect that there is a well formed packet. Try to
  # read the whole thing. If the full packet isn't there,
  # return a null string and move on. 
  # If the packet is found, truncate "line" so that the 
  # packet is no longer being outputted.
  #
  # \param content  the accumulated text to process starting with directive
  method buildPacket {content} {
  #puts "Build packet with {$content}"
    set incomplete 1

    # find first and second word boundaries 
    set b1 [string first { } $content 0]
    if {$b1 == -1} return
    set b2 [string first { } $content [expr $b1+1]]
    if {$b2 == -1} return
    
    set pktSize [string trim [string range $content $b1 $b2] { \n}]
    if {![string  is integer -strict $pktSize]} {
      # Not a packet at all! Let's create new content with size interpolated.
      set directive [string trim [string range $content 0 $b1]]
      set remainder [string range $content [expr $b1+1] end]
      #puts "directive: $directive"
      #puts "remainder $remainder"
      set size [string length $remainder]
      set content "$directive $size $remainder"
      #puts  "revised contents: '$content'"
      
      set b1 [string first { } $content 0]
      set b2 [string first { } $content [expr $b1+1]]
      set pktSize [string trim [string range $content $b1 $b2] { \n}]
    }
    set totalLength [string length $content]
    set remChars [expr $totalLength - ($b2+1)]
    
    if {$remChars >= $pktSize} {
       set b3 [expr {$b2+$pktSize}]
       lappend parsedLine [$self extractFirstWord $content] 
       lappend parsedLine [string range $content [expr $b2+1] $b3] 
       lappend parsedLine [string range $content [expr $b1+1] [expr $b2-1]] 

       set incomplete 0
       set content [string range $content [expr $b3+1] end] 
       #puts "Parsed: $parsedLine"
       return $parsedLine
    } else {
       return ""
    }

  }

  ## Parse msg that we have identified NOT to begin with a directive
  #
  # Deal with non packet output... we simply output
  # everything we have up until a valid directive is 
  # found. If we find a directive, output everything up to that
  # directive, pop the outputted msg from the front of line,
  # return.  This is an attempt to handle malformed output that
  # did not have a newline character.
  #
  # \param content  the accumulated text to process
  method handleNonPacket {content} {
  
    # handle the scenario when the line contains a directive somewhere
    # in it
    foreach dir $legalDirectives {
      set index [string first $dir $content]
      if {$index != -1} {
        # we found a directive, output everything up to that directive
        set msg [string range $content 0 [expr $index-1]]
        set content [string range $content $index end]

        return  [list OUTPUT $content [string length $content]]
      }
    }

    # if we are here then we didn't find any directives, output the whole line
    set result [list OUTPUT $content [string length $content]]
    return $result
  } 

  # Determine if there word is a legal directive
  method isLegalDirective {word} {
    return [expr {$word in $legalDirectives}]
  }

  # Jump-table of sorts for passing various 
  # handlers to their handlers
  method handleMessage {parsedLine} {

    set directive [lindex $parsedLine 0]
    set msg [lindex $parsedLine 1]
    set dirId [dict get $directiveMap $directive]
    set result {}
    #puts "$dirId"
    switch $dirId {
      0 { set result [$self handleError $msg ] }
      1 { set result [$self handleLog $msg ]}
      2 { set result [$self handleWarning $msg ] }
      3 { set result [$self handleTclCommand $msg ]}
      4 { set result [$self handleOutput $msg] }
      5 { set result [$self handleDebug $msg] }
    } 

    return $result
  }

  method handleError {str} {
    return [$options(-actionbundle) handleError $str]
  }

  method handleLog {str} {
   return [$options(-actionbundle) handleLog $str]
  }

  method handleWarning {str} {
    return [$options(-actionbundle) handleWarning $str]
  }

  method handleDebug {str} {
    return [$options(-actionbundle) handleDebug $str]
  }

  method handleOutput {str} {
    return [$options(-actionbundle) handleOutput $str]
  }

  method handleTclCommand {str} {
    return [$options(-actionbundle) handleTclCommand $str]
  }
}

