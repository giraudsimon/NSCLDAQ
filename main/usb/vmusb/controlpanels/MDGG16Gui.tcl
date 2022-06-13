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


package provide mdgg16gui 1.0

package require snit
package require Tk
package require Utils
package require mdgg16proxy
package require TransientLabel

namespace eval MDGG16ChannelNames {
  variable chan0 Ch0
  variable chan1 Ch1
  variable chan2 Ch2
  variable chan3 Ch3
  variable chan4 Ch4
  variable chan5 Ch5
  variable chan6 Ch6
  variable chan7 Ch7
  variable chan8 Ch8
  variable chan9 Ch9
  variable chan10 Ch10
  variable chan11 Ch11
  variable chan12 Ch12
  variable chan13 Ch13
  variable chan14 Ch14
  variable chan15 Ch15

}


## @brief A column of checkbuttons with 17 rows
#
# There are 16 individually configurable buttons and one button that will toggle
# all of the buttons together. This view that is not entirely devoid
# of logic in that it can update whether any of its variables have been altered
# since a previous sync reset. The widget is intended to be used with another
# presenter-like widget. In the case of the MDGG16Control, it is managed by
# another view that serves as a proxy to the real presenter.
# 
snit::widget CheckbuttonColumn {

  hulltype ttk::frame  ;##!< make sure we can set the style
  delegate option * to hull ;##!< make sure we can set the style

  option -presenter -default {} -configuremethod SetPresenter
  option -stylename -default {} -configuremethod SetStyle

  ## "private" variables that maintain the state of the checkbuttons
  variable _bit0
  variable _bit1
  variable _bit2
  variable _bit3
  variable _bit4
  variable _bit5
  variable _bit6
  variable _bit7
  variable _bit8
  variable _bit9
  variable _bit10
  variable _bit11
  variable _bit12
  variable _bit13
  variable _bit14
  variable _bit15

  variable _outOfSync ;##! maintains whether at least one widget has changed
                      ;#  since last sync

  variable _widgets ;##! convenient list of 16 individual widgets

  ## @brief Construct the object, initialize, and parse options
  #
  constructor {args} {
    set _widgets [list]

    $self configurelist $args

    $self SetOutOfSync 1

    # By default we will set these to 1. This is just a choice of
    # initialization. In the presence of a real device associated with this,
    # the values should be updated immediately after connection is established.
    for {set ch 0} {$ch < 17} {incr ch} {
      set _bit$ch 1
      trace add variable [myvar _bit$ch] write [mymethod BitChanged]
    }

    $self BuildGUI
    $self UpdateStyle [$self cget -style]
  }

  ## @brief Assemble the widgets
  #
  #
  method BuildGUI {} {
    # intialize checkbuttons
    for {set ch 0} {$ch < 16} {incr ch} {
      lappend _widgets [ttk::checkbutton $win.bit$ch -variable [myvar _bit$ch]]
    }
    set sep [ttk::separator $win.separator -orient horizontal]
    ttk::checkbutton $win.bit16 -variable [myvar _bit16]
    trace add variable [myvar _bit16] write [mymethod Synchronize]

    # grid the widgets
    for {set row 0} {$row<16} {incr row} {
      grid [lindex $_widgets $row] -sticky ew -padx 4 -pady 4
    }
    grid $sep -sticky ew
    grid $win.bit16 -sticky ew -padx 4 -pady 4

    # all rows and columns are allowed to stretch
    grid rowconfigure $win [Utils::sequence 0 [llength $_widgets] 1] -weight 1 
    grid columnconfigure $win 0 -weight 1
  }

  ## @brief Set a specific bit to a value
  #
  # This does not check for valid parameter values and leaves it to the caller
  # to make sure that they are reasonable.
  #
  # @param index  bit index (must be in range [0,7])
  # @param val    value  (must be either 0 or 1)
  method SetBit {index val} {
    set _bit$index $val
  }

  ## @brief Retrieve the index-th bit value
  # 
  # There is no parameter checking in this so it is up to the caller to pass in
  # a good bit index.
  #
  # @param index  index of bit (must be in range [0,7])
  #
  # @returns value of bit
  method GetBit {index} {
    return [set _bit$index]
  }


  ## @brief It is possible that a presenter could control this.
  #
  # There is not gaurantee that there will be a presenter but it is allowed.
  #
  method SetPresenter {opt val} {
    set options($opt) $val
  }

  ## @brief Callback for when -stylename is modified
  #
  # This is just a dispatcher to the UpdateStyle method. It does set the option
  # to the value though.
  #
  # @param opt    option name (should be -stylename)
  # @param val    value 
  #
  method SetStyle {opt val} {
    $self UpdateStyle $val
    set options($opt) $val
  }

  ## @brief Propagate a style change to the various widgets
  #
  # @param stylename  prefix name for a ttk style
  #
  method UpdateStyle {stylename} {
    if {[llength $_widgets]>0} {
      foreach w $_widgets {
        $w configure -style "$stylename.TCheckbutton"
      }
      $win.bit16 configure -style "$stylename.TCheckbutton"
      $self configure -style "$stylename.TFrame"
    }
  }

  ## @brief Callback for whenever bit 16 is written to
  #
  # When bit 16 is written to, it triggers all of the checkbuttons to be set
  # identically. It is like a "select all" or "deselect all"
  #
  # @param name1  unused
  # @param name2  unused
  # @param op     unused
  #
  method Synchronize {name1 name2 op} {
    # get the state of the variable
    set state [set [$win.bit16 cget -variable]]

    # set the other widget values to be the same 
    foreach w $_widgets {
      set [$w cget -variable] $state
    }
  }


  ## @brief Callback for whenever a bit is written to
  #
  # This is the callback of a trace set on all of the _bit# variables associated
  # with the instance. When called, this will make sure to flag that the view is
  # out of sync. 
  #
  # @param name1  unused
  # @param name2  unused
  # @param op     unused
  #
  method BitChanged {name1 name2 op} {
    if {!$_outOfSync} {
      $self SetOutOfSync 1
    }
  }

  ## @brief Sets the flag that indicates whether the view has changed since last
  #         time _outOfSync was reset.
  # 
  #  @param state   boolean indicating whether out of sync or not
  #
  #
  method SetOutOfSync {state} {
    set _outOfSync [string is true $state]
  }

  ## @brief Retreive fully qualified name of the _outOfSync variable for
  #         potentially registering a callback for it outside of this scope
  #
  # @returns qualified variable name of _outOfSync
  method GetOutOfSyncVar {} { return [myvar _outOfSync]}
}


##########################
#

## @brief Column of specialized ttk::entry widgets
#
# In the single column of ttk::entry widget this produces, there are 16 rows. In
# the 17th row, there is a single ttk::label that says "Common". The column that
# this creates is intended to be used alongside a CheckbuttonColumn. The
# variables controlled by the ttk::entry are defined in the MDGG16ChannelNames
# namespace. The top row corresponds to channel 0, the bottom row to channel
# 15, and the rows in between map linearly. 
#
# The entry widgets have been specialized to allow specialization of the values
# to be a name while not allowing an empty string. If the user specifies
# whitespace only in the entry, the value will fall back to a default value that
# is Ch#.
#
# In general this is a standalone megawidget that needs no presenter. It is just
# a collection of widgets. 
snit::widget NameColumn {

  ## @brief  Construct and assemble gui
  #
  constructor {} {
    set _rows [list]
    $self BuildGUI
  }

  ## @brief Assemble widgets into a gui
  #
  # This grids all of the widgets and makes them stretchable in the horizontal
  # direction.
  #
  method BuildGUI {} {
    for {set ch 0} {$ch < 16} {incr ch} {
      lappend _rows [ttk::entry $win.name$ch -textvariable MDGG16ChannelNames::chan$ch \
        -width 20\
        -validate focus -validatecommand [mymethod ValidateName %P] \
        -invalidcommand [mymethod ResetChannelName %W]]
    }
    lappend _rows [ttk::label $win.common -text "Common"]

    foreach row $_rows {
      grid $row -sticky ew -padx 4 -pady 4
    }

    grid rowconfigure $win [Utils::sequence 0 [llength $_rows] 1] -weight 1 
    grid columnconfigure $win 0 -weight 1
  }

  ## @brief check whether a channel name contains non-whitespace characters
  # 
  # this is called when a channel entry loses focus
  #
  # @param name   candidate string 
  #
  # @return boolean
  # @retval 0 - string was empty or all whitespace
  # @retval 1 - otherwise
  method ValidateName {name} {
    set name [string trim $name]
    set good [expr [string length $name]!=0]

    return $good
  }

  ## @brief reset channel to a simple string
  #
  # typically called with validatename returns false. it will set the string
  # to "ch#".
  #
  # @returns ""
  method ResetChannelName {widget} {
    set str [$widget cget -textvariable]
    regexp {^.*(\d+)$} $widget match ch
    set $str "Ch$ch"
  }
}

############################
#

## @brief The main megawidget
#
# This is just a NameColumn and 4 CheckbuttonColumns with some buttons. The
# buttons at the bottom provide the ability to update from the device or commit
# the state of the view to the device. The actual functionality of these buttons
# is suggested, but it is the presenter attached to the instance that actual
# determines what happens when these buttons are pressed because this will only
# forward the button press event to the presenter. 
#
# An instance of this class will trace the variables of the CheckbuttonColumns
# that it owns and use the values of those to maintain whether the update button
# should be active or not.
# 
# Without a presenter object, this will keep track of whether synchronization
# has been lost but there will never be any way to reset it to be synchronized.
# In fact, the megawidget will look usable, but in reality nothing will happen
# when the commit and update button are pressed.
#
snit::widget MDGG16View {
  hulltype ttk::frame

  option -presenter -default {} ;##!< handles logic for commit and update

  delegate option * to hull

  component _colNames ;##!< name column widget
  component _colA     ;##!< column A
  component _colB     ;##!< column B
  component _colC     ;##!< column C
  component _colD     ;##!< column D
  component _syncLbl  ;##!< out of sync label
  component _updateButton 
  component _status ;##!< status label

  variable _outOfSyncAVar ;##!< name of colA's out of sync state variable
  variable _outOfSyncBVar ;##!< name of colB's out of sync state variable
  variable _outOfSyncCVar ;##!< name of colC's out of sync state variable
  variable _outOfSyncDVar ;##!< name of colD's out of sync state variable

  ## @brief Construct, parse options, and assemble
  #
  # @param args   option value pairs
  #
  constructor {args} {
    $self configurelist $args

    $self BuildGUI
    $self SetUpSync
  }

  ## @brief Destruct
  #
  # Destroy all of the subcomponents.
  destructor {
    catch {destroy $_colNames}
    catch {destroy $_colA}
    catch {destroy $_colB}
    catch {destroy $_colC}
    catch {destroy $_colD}
    catch {destroy $_updateButton}
    catch {destroy $_status}
  }

  ## @brief Assembles the widgets into a unified megawidget
  #
  method BuildGUI {} {

    # Big title at top of widget
    set title [ttk::frame $win.titleFrame]
    set titleLbl [ttk::label $title.title -text "MDGG-16 Controls" -style Title.TLabel]
    set _syncLbl [ttk::label $title.syncLbl \
      -text "OUT OF SYNC...\n PRESS COMMIT OR UPDATE TO RESYNC" \
      -style OutOfSync.TLabel]
    grid $_syncLbl -sticky ew 
    grid $titleLbl -sticky ew
    grid columnconfigure $title 0 -weight 1
    
    # The titles for each column
    set header [ttk::frame $win.headers -style "Header.TFrame"]
    ttk::label $header.names -text "Names" -width 24 -style Header.TLabel
    ttk::label $header.orA -text "OR A" -style Header.TLabel
    ttk::label $header.orB -text "OR B" -style Header.TLabel
    ttk::label $header.orC -text "OR C" -style Header.TLabel
    ttk::label $header.orD -text "OR D" -style Header.TLabel
    grid $header.names $header.orA $header.orB $header.orC $header.orD -sticky nsew -padx 4 -pady 4
    grid columnconfigure $header 0 -weight 4 -uniform a
    grid columnconfigure $header {1 2 3 4} -weight 1 -uniform a

    # The columns
    set cols [ttk::frame $win.cols]
    install _colNames using NameColumn $cols.colNames
    install _colA using CheckbuttonColumn $cols.colA -stylename "Even"
    install _colB using CheckbuttonColumn $cols.colB -stylename "Odd"
    install _colC using CheckbuttonColumn $cols.colC -stylename "Even"
    install _colD using CheckbuttonColumn $cols.colD -stylename "Odd"
    grid $_colNames $_colA $_colB $_colC $_colD -sticky nsew
    grid rowconfigure $cols {0 1 2 3 4} -weight 0
    grid columnconfigure $cols 0 -weight 4 -uniform a
    grid columnconfigure $cols {1 2 3 4} -weight 1 -uniform a

    # the buttons
    set buttons [ttk::frame $win.buttons]
    ttk::button $buttons.commit -text "Commit to Device" -command [mymethod Commit]
    set _updateButton [ttk::button $buttons.update -text "Update from Device" \
                                                   -command [mymethod Update]]

#    set _status [ttk::label $win.status -text {}]
    set _status [TransientLabel $win.status -timerlength 2000 -text {}]
    grid $buttons.commit $buttons.update -sticky ew
    grid columnconfigure $buttons {0 1} -weight 1

    grid $title -sticky new
    grid $header -sticky ew
    grid $cols -sticky nsew
    grid $buttons -sticky new
    grid $_status -sticky sew -pady {4 0}
    grid rowconfigure $win {0 1 2 3} -weight 1
    grid columnconfigure $win 0 -weight 1

  }
  
  ## @brief Sets traces on the variables managing sync state
  #
  method SetUpSync {} {
    set _outOfSyncAVar [$_colA GetOutOfSyncVar]
    trace add variable $_outOfSyncAVar write [mymethod SyncChanged]
    set _outOfSyncBVar [$_colB GetOutOfSyncVar]
    trace add variable $_outOfSyncBVar write [mymethod SyncChanged]
    set _outOfSyncCVar [$_colC GetOutOfSyncVar]
    trace add variable $_outOfSyncCVar write [mymethod SyncChanged]
    set _outOfSyncDVar [$_colD GetOutOfSyncVar]
    trace add variable $_outOfSyncDVar write [mymethod SyncChanged]

  }

  ## @brief Forward a commit button pressed event
  # 
  # If no presenter has been set, then this is a no-op
  #
  method Commit {} {
    set pr [$self cget -presenter]
    if {$pr ne {}} {
      $pr Commit
    }
  }

  ## @brief Forward an update button pressed event
  # 
  # If no presenter has been set, then this is a no-op
  #
  method Update {} {
    set pr [$self cget -presenter]
    if {$pr ne {}} {
      $pr Update 
    }
  }

  ## @brief Access value of specific bit for a certain column
  #
  # Provides that way to provide access to bits of the sub widgets without
  # needed to know much about them other than which column.
  #
  # @param col    index of checkbutton column (0=leftmost, ..., 3=rightmost)
  # @param ch     index of bit in column
  #
  # @warning There is no check for sanity of parameters passed into this. It is
  #          the callers responsibility to make sure these are sensible.
  method GetBit {col ch} {
    set widget [$self MapColumnToWidget $col]
    return [$widget GetBit $ch]
  }

  ## @brief Set a value for a bit in a column
  #
  # Analogous to the GetBit method except that in this case we write the bit.
  #
  # @param col    index of checkbutton column (0=leftmost, ..., 3=rightmost)
  # @param ch     index of bit in column
  # @param val    value to write (should be 0 or 1)
  #
  method SetBit {col ch val} {
    set widget [$self MapColumnToWidget $col]
    return [$widget SetBit $ch $val]
  }

  ## @brief Mechanism to get name of widget forming a specific column
  #
  # @param col  checkbutton column index (0=leftmost, ..., 3=rightmost)
  #
  # @returns fully-qualified name of CheckbuttonColumn widget
  #
  # @throws error if col is out of range
  method MapColumnToWidget {col} {
    return [dict get [dict create 0 $_colA 1 $_colB 2 $_colC 3 $_colD] $col]
  }

  ## @brief Callback for when any of the button columns go out of sync
  #
  # This provides the mechanism for enabling and disabling the update button. 
  #
  # @param name1  unused
  # @param name2  unused
  # @param op     unused
  #
  method SyncChanged {name1 name2 op} {
    set outA [set $_outOfSyncAVar]
    set outB [set $_outOfSyncBVar]
    set outC [set $_outOfSyncCVar]
    set outD [set $_outOfSyncDVar]
    if { $outA || $outB || $outC || $outD } {
      $_updateButton state !disabled
      grid $_syncLbl -row 0 
    } else {
      $_updateButton state disabled
      grid remove $_syncLbl
    }
  }

  method SetOutOfSync {state} {
    $_colA SetOutOfSync $state
    $_colB SetOutOfSync $state
    $_colC SetOutOfSync $state
    $_colD SetOutOfSync $state
  }

  method SetStatus {message} {
    $_status configure -text $message
  }
}

##############################################################################

## @brief The logic for the MDGG16View
#
# Provides the logic for handling when the commit or update buttons are pressed
# in the the MDGG16View. This maintains a reference to a view and to a device
# handle. The expected arguments here are for the view to be an MDGG16View and
# for the handle to be an MDGG16Proxy. However, duck typing applies and any
# satisfactory substitute can be used in place of any of these.
#
# It is important to understand that neither the view nor the handle are owned
# by this snit::type.
snit::type MDGG16Presenter {

  option -view -default {} -configuremethod SetView ;#!< the view
  option -handle -default {} -configuremethod SetHandle ;#!< the handle

  variable _clearStatusOpId -1 ; ##!< Previously scheduled status clear op

  ## @brief Parse options and construct
  #
  constructor {args} {
    $self configurelist $args
  }

  ## @brief Handler for when the view state is to be transmitted to device
  #
  # First commits the mask and then reads the state back from the device
  #
  method Commit {} {
    $self CommitMask
    $self UpdateViewFromModel
  }

  ## @brief Read the state from the device and synchronize to the view
  #
  method Update {} {
    $self UpdateViewFromModel
  }

  ## @brief Read the state from the device and synchronize to the view
  #
  # @throws error if no handle exists
  # @throws error if no view exists
  # @throws error if communication fails
  method UpdateViewFromModel {} {

    set bits [$self RetrieveAndDecodeMask AB]
    $self SetBitsForColumn 0 [lrange $bits 0  15]
    $self SetBitsForColumn 1 [lrange $bits 16 31]

    set bits [$self RetrieveAndDecodeMask CD]
    $self SetBitsForColumn 2 [lrange $bits 0  15]
    $self SetBitsForColumn 3 [lrange $bits 16 31]

    $self UpdateOutOfSyncState 0
  }

  ## @brief Write the state of the view to the device
  #
  # @throws error if no handle exists
  # @throws error if no view exists
  #
  method CommitMask {} {
    $self ThrowIfNoHandle ${type}::CommitMask
    set handle [$self cget -handle]

    ### Logical OR AB
    set mask [$self RetrieveAndEncodeBits {0 1}]
    $handle SetLogicalORAB $mask

    ### Logical OR CD 
    set mask [$self RetrieveAndEncodeBits {2 3}]
    $handle SetLogicalORCD $mask
  }

  ## @brief Saves view state to a file.
  #
  # This is almost identical to the commit method except that it 
  # writes the mask values to a file instead of to a device. This is invoked
  # the MDGG16GuiApp's "Save as..." menu button at the present moment.
  #
  # Produces a file that looks kind of like:
  # or_a  123
  # or_b  123
  # or_c  123
  # or_d  123
  #
  # Where the numbers are the calculated bit masks in decimal.
  #
  # @param path   name of file to save to
  #
  # @throws error if now view exists
  method SaveCurrentStateToFile {path} {

    # it makes no sense to do this if there is no view
    $self ThrowIfNoView ${type}::SaveCurrentStateToFile

    # open the file
    set outfile [open $path w+]

    chan puts $outfile "Configuration file for MDGG16Control"
    chan puts $outfile [clock format [clock seconds]]

    set view [$self cget -view]

    ### Logical OR AB
    set mask [$self RetrieveAndEncodeBits {0 1}]
    set or_a [expr $mask & 0xffff]
    set or_b [expr ($mask>>16) & 0xffff]
    chan puts $outfile "or_a $or_a"
    chan puts $outfile "or_b $or_b"

    ### Logical OR CD 
    set mask [$self RetrieveAndEncodeBits {2 3}]
    set or_c [expr $mask & 0xffff]
    set or_d [expr ($mask>>16) & 0xffff]
    chan puts $outfile "or_c $or_c"
    chan puts $outfile "or_d $or_d"

    # save names
    for {set ch 0} {$ch < 16} {incr ch} {
      chan puts $outfile [format "%2d : %s" $ch [set ::MDGG16ChannelNames::chan$ch]]
    }

    close $outfile
  }

  method LoadStateFromFile {path} {
    # open the file
    set infile [open $path r]

    chan gets $infile line
    chan gets $infile line 

    # read AB mask values
    chan gets $infile line
    set line [string trim $line " "]
    set maska [lindex [split $line " "] 1]
    chan gets $infile line
    set line [string trim $line " "]
    set maskb [lindex [split $line " "] 1]
    set maskab [expr ($maskb<<16)|$maska]

    set bits [$self DecodeMaskIntoBits $maskab]
    $self SetBitsForColumn 0 [lrange $bits 0  15]
    $self SetBitsForColumn 1 [lrange $bits 16 31]

    # read CD mask values
    chan gets $infile line
    set line [string trim $line " "]
    set maskc [lindex [split $line " "] 1]
    chan gets $infile line
    set line [string trim $line " "]
    set maskd [lindex [split $line " "] 1]
    set maskcd [expr ($maskd<<16)|$maskc]

    set bits [$self DecodeMaskIntoBits $maskcd]
    $self SetBitsForColumn 2 [lrange $bits 0  15]
    $self SetBitsForColumn 3 [lrange $bits 16 31]

    # read names if they exist
    set pattern {^\s*(\d+) : (\w*)$}
    while {1} {
      chan gets $infile line
      if {[eof $infile]} break

      set matches [regexp -inline -- $pattern $line]
      if {[llength $matches] == 3} {
        set index [lindex $matches 1]
        set name  [lindex $matches 2]
        set ::MDGG16ChannelNames::chan$index $name
      }
    }

    close $infile
  }

  #############################################################################
  #
  # UTILITY METHODS
  #

  ## @brief Read mask from device and transform into list of bits
  #
  # This will read the specified mask from the device and parse it into a
  # representation in bits. The list will contain 32 elements (1 for each bit)
  # and the order will be least significant to most significant.
  #
  # @param name   mask name (either AB or CD)
  #
  # @returns 32 element list of 0s and 1s
  #
  # @throws error if no device handle exists
  method RetrieveAndDecodeMask {name} {
    $self ThrowIfNoHandle ${type}::RetrieveAndDecodeMask

    set handle [$self cget -handle]
    set mask [$handle GetLogicalOR$name]

    # split the mask into a list of bits
    return [$self DecodeMaskIntoBits $mask]
  }

  ## @brief Write list of bits to checkbuttons in a given column
  #
  # @param col  column number (either 0, 1, 2, or 3)
  # @param bits list of 0s and 1s (must be 16 elements long)
  #
  # @throws error if no view exists
  method SetBitsForColumn {col bits} {

    $self ThrowIfNoView ${type}::SetBitsForColumn

    set view [$self cget -view]
    for {set bit 0} {$bit < 16} {incr bit} {
      $view SetBit $col $bit [lindex $bits $bit]
    }
  }

  ## @brief Update the out of sync value in value
  #
  # Typcially this is just called to let the view know that it has recently been
  # synced.
  #
  # @param state  boolean value indicating whether out of sync
  #
  # @throws error if no view exists
  method UpdateOutOfSyncState {state} {
    $self ThrowIfNoView ${type}::UpdateOutOfSyncState 

    [$self cget -view] SetOutOfSync $state 
  }


  ## @brief The opposite of RetrieveAndDecodeMask
  # 
  # Reads the bits from two columns and then forms them into a 32-bit unsigned
  # integer. The column specified as element 0 of the argument forms the lower
  # 16-bits of the integer and the second element forms the upper 16-bits.
  #
  # @param cols   2-element list containing which two lists to use
  #
  # @return 32-bit unsigned integer 
  method RetrieveAndEncodeBits {cols} {
    $self ThrowIfNoView ${type}::RetrieveAndEncodeBits

    set view [$self cget -view]

    set bits [list]
    foreach col $cols {
      for {set index 0} {$index < 16} {incr index} {
        lappend bits [$view GetBit $col $index]
      }
    }

    # turn list of bits into a number
    return [$self EncodeBitsIntoMask $bits]
  }


  ## @brief Split an integer into a list of bits
  #
  # Given an integer, convert it to a list of 0s and 1s that represent it. Split
  # the bits up and form a list. For example, passing 100 (0x64) into this method, the
  # result will be {0 0 1 0 0 1 1 0}
  #
  # @returns list of 8 bits (least significant bit first)
  method DecodeMaskIntoBits {mask} {
    set bits [list]

    # interpret mask as a 32-bit unsigned integer
    set byteValue [binary format iu1 $mask]

    # convert integer into representation of bits as a string
    set count [binary scan $byteValue b32 binRep]

    # split each character up to form a list of bits
    return [split $binRep {}]
  }

  ## @brief Turn a list of bits into an equivalent integer
  #
  # This is the opposite operation of the DecodeMaskIntoBits. Given a list i
  # {0 0 1 0 0 1 1 0}, the method will return a value of 100.
  #
  # @param list of bits (least significant bit first)
  #
  # @returns an integer
  method EncodeBitsIntoMask {bits} {
    set mask 0

    # collapse list of bits into a single word by removing spaces
    set binRepStr [join $bits {}]

    # convert string representation of bits into an actual byte string
    set binByte [binary format b32 $binRepStr]

    # interpret the byte as an 32-bit unsigned integer
    set count [binary scan $binByte iu1 mask]

    # becuase the number if signed and padded, we mask out upper bits
    return $mask
  } 

  ## @brief Callback for a "configure -view" operation
  # 
  # Performs the handshake required when the view is set. A new view is passed
  # $self as the value to its -presenter option. If a handle exists, it is
  # appropriate to update the state of the view from it.
  #
  # @param opt    option name (should be -view)
  # @param val    value (name of view)
  method SetView {opt val} {
  # store the new view (opt="-view", val = new view name)
    set options($opt) $val
    $val configure -presenter $self

    # we have a handle already, then update!
    set handle [$self cget -handle]
    if {$handle ne {}} {
      $self UpdateViewFromModel
    }
  }

  ## @brief Callback for a "configure -handle" operation
  #
  # Sets the -handle option to the new value and also updates the view from it
  # if the -view option is set.
  #
  # @param opt  option name (should be -handle)
  # @param val  value (name of handle)
  #
  method SetHandle {opt val} {
  # store the new handle (opt="-handle", val = new handle name)
    set options($opt) $val

    # we have a view already, then update!
    set view [$self cget -view]
    if {$view ne {}} {
      $self UpdateViewFromModel
    }
  }

  ## @brief Throws an error if no handle exists 
  #
  # @param context  method name to indicate what called this.
  #
  # @throws error
  method ThrowIfNoHandle {context} {
    if {[$self cget -handle] eq {}} {
      set msg "$context "
      append msg {Cannot access model because it does not exist.}
      return -code error $msg
    }
  }

  ## @brief Throws an error if no handle exists 
  #
  # @param context  method name to indicate what called this.
  #
  # @throws error
  method ThrowIfNoView {context} {
    if {[$self cget -view] eq {}} {
      set msg "$context "
      append msg {Cannot access model because it does not exist.}
      return -code error $msg
    }
  }

}

