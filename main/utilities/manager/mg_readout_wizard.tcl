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
# @file mg_readout_wizard.tcl
# @brief Wizard to create readout programs.
# @author Ron Fox <fox@nscl.msu.edu>
#

if {[array names env DAQTCLLIBS] ne ""} {
    lappend auto_path $env(DAQTCLLIBS)
}
package require Tk
package require snit

package require programs
package require sequence
package require containers

##
# Provides a wizard that creates all of the stuff to make a readout program work.
# This is not only the Readout program itself but the REST clients that
# are needed to start it.  Each Readout program named xxx
# will create the following programs.
#
#  -  xxx the readout itself.
#  -  xxx_init - initializes the hardware.
#  -  xxx_setrun - sets the run number from the KV store.
#  -  xxx_settitle - sets the title from the KV Store.
#  -  xxx_beginrun - Starts the run.
#  -  xxx_endrun   - Ends the run.
#  -  xxx_shutdown - Shuts the readout down.
#
#  If the following sequences don't exist they are created
#
#   -  bootreadout - triggered by the BOOT transition and xxx is added to that sequence.
#   -  initreadout - triggered by the  HWINIT transition and xxx_init is added to that sequence.
#   -  beginreadout- triggered by the BEGIN transition, xxx_setrun, xxx_settitle
#                     and xxx_beginrun are added to that sequence.
#   -  endreadout  - Triggered by the END transtion, xxx_endrun is added to it.
#   -  shutdownreadout - Triggered by SHUTDOWN< xxx_shutdown is added to it.
#
#  For each Readout we need the following:
#   *  Type of Readout - XIA/DDAS-12+, VMUSB, CCUSB, Customized
#   *  Container - can be none or any of the defined containers.
#   *  Host
#   *  Working directory.
#   *  IF customized, the location of the Readout program.
#   *  Source id  - for the event builder.
#   *  ring   - Name of ring buffer.
#   *  Rest service name (defaults to ReadoutREST)
#   IF XXUSBReadout:
#    ctlconfig, daqconfig
#   IF XIADDAS-12+:
#     sort host, sort ring, sort window, fifo threshold, buffersize, infinity clock,
#     clock multiplier, scaler readout period.
#   If Customized
#     Readout filename.
#     Any options the user wants to add.
#
#   Environment variables the program will have... in addition to those set up by
#   the container start script.
#
#  Note that prior to DDAS readout prior to V12 should use customized and select
#  $DAQBIN/DDASReadout configuring it manually using environment variables etc.
#

#==============================================================================
#   Utility widgets:
#

##
# @class NameValueList
#    Form to prompt for a set of name/value items.
#    These are not assumed to be ordered.
#
# OPTIONS
#   *  -items - List of name vale pairs.  It is legal for there to be no
#               value.
#
#
snit::widgetadaptor NameValueList {
      option -items -configuremethod _stockList  -cgetmethod _getList
      
      constructor args {
          installhull using ttk::frame
          
          # The list box and scrollbars.
          
          ttk::frame $win.list
          listbox    $win.list.listbox    \
              -xscrollcommand [list $win.list.hscroll set]     \
              -yscrollcommand [list $win.list.vscroll set]     \
              -selectmode single
          ttk::scrollbar $win.list.vscroll -orient vertical \
              -command [list $win.list.listbox yview] 
          ttk::scrollbar $win.list.hscroll -orient horizontal \
              -command [list $win.list.listbox xview]
          
          grid $win.list.listbox $win.list.vscroll -sticky nsew
          grid $win.list.hscroll              -sticky ew
          
          # The entries and add button.
          
          ttk::frame $win.entries
          ttk::entry $win.entries.name
          ttk::label $win.entries.namelabel -text Name
          ttk::entry $win.entries.value
          ttk::label $win.entries.valuelabel -text Value
          ttk::button $win.entries.add -text Add -command [mymethod _addItem]
          
          grid $win.entries.name $win.entries.namelabel   -padx 3 -sticky nsew
          grid $win.entries.value $win.entries.valuelabel -padx 3 -sticky nsew
          grid $win.entries.add -sticky w
          
          grid $win.list  -sticky nsew
          grid $win.entries -sticky nsew
          
          # The context menu which posts on <Button-3>
          
          menu $win.context -tearoff 0
          $win.context add command -label Delete -command [mymethod _deleteEntry]
          $win.context add command -label Edit   -command [mymethod _editEntry]
          
          bind $win.list.listbox <Button-3> [mymethod _contextMenu %x %y %X %Y]
          
          
          $self configurelist $args
      }
      #-------------------------------------------------------------------------
      #  Configuration methods.
      #
      
      ##
      # _stockList
      #   Called to configure the items.  Each item is a one or two element
      #   List.  If two elements we put quotations around the second element
      #   to ensure that the list is valid.
      #
      method _stockList {name value} {
           
           set w $win.list.listbox
           
           $w delete  0 end
           foreach item $value {
               set optname [lindex $item 0]
               set v ""
               set item [lrange $item 1 end]
               if {$item ne ""} {
                 set v "\"$item\""
               }
               $w insert end "$optname $v"
           }
           
      }
      ##
      # _getList
      #    Returns the value of the list.
      #
      method _getList {name} {
           set w $win.list.listbox
           
           set contents [$w get 0 end]
           set result [list]
           foreach entry $contents {
              if {[llength $entry] == 1} {
                  lappend result $entry
              } else {
                 set pair [list [lindex $entry 0] [lrange $entry 1 end]]
                 lappend result $pair
              }
           }
            
           
           
           return $result
      }
      #------------------------------------------------------------------------
      # Event handling.
      
      ##
      # _addItem
      #   Called when the add button is clicked.  There must be a name
      #   if there is a value then a two element list is added to the list box.
      #
      method _addItem {} {
           set l $win.list.listbox
           set n $win.entries.name
           set v $win.entries.value
           
           set name [string trim [$n get]]
           set value [string trim [$v get]]
           
           
           if {$name ne ""} {
               set item [list $name]
               if {$value ne ""} {
                   lappend item $value
               }
               $l insert end $item
           }
      }
      
      ##
      # _contextMenu
      #   In response to a right click.
      #   - If the mouse is over an entry, select it and pop up the
      #     menu.
      #   -  Otherwise do notyhing
      #
      # @param x - window relative pointer x.
      # @param y - window relative pointer y.
      # @param X,Y - screen relative pointer x/y
      #
      method _contextMenu {x y X Y} {
           set l $win.list.listbox
           set index [$l nearest $y]
           
           $l selection clear 0 end
           $l selection set $index $index
           $win.context post $X $Y
      }
      ##
      # _deleteEntry
      #    Delete the selected element...and unpoast the context menu.
      #
      method _deleteEntry {} {
           set l $win.list.listbox
           set selIndex [$l curselection]
           if {$selIndex ne ""} {
              $l delete $selIndex $selIndex
           }
           $win.context unpost
      }
      ##
      # _editEntry
      #   Edits the selected entry.  The entry is loaded into the
      #   Name, and value entries and then deleted from the list.
      #   Note that the menu is unposted.
      method _editEntry {} {
           set l $win.list.listbox
           
           set selindex [$l curselection]
           if {$selindex ne ""} {
               set selection [$l get $selindex]
               $win.entries.name delete 0 end
               $win.entries.value delete 0 end
               
               $win.entries.name  insert 0 [lindex $selection 0]
               $win.entries.value insert 0 [lrange $selection 1 end]
               
               $l delete $selindex $selindex
           }
           $win.context unpost
      }
}
      
##
# OrderedValueList
#
#   As the name implies, this widget provides a mechanism to enter, and edit
#   a list of values where order important.
#
#  OPTIONS
#  *  -values list of values in the listbox (set/get).
#
snit::widgetadaptor OrderedValueList {
    option -values -default [list] -configuremethod _cfgValues \
           -cgetmethod _cgetValues
    
    constructor args {
        installhull using ttk::frame
        
        # List box and scrollbars.
        
        ttk::frame $win.list
        listbox    $win.list.listbox   -selectmode single    \
            -xscrollcommand [list $win.list.hscroll set]     \
            -yscrollcommand [list $win.list.vscroll set]
        ttk::scrollbar $win.list.vscroll -orient vertical  \
            -command [list $win.list.listbox yview]
        ttk::scrollbar $win.list.hscroll -orient horizontal \
            -command [list $win.list.listbox xview]
        
        grid $win.list.listbox $win.list.vscroll -sticky nsew
        grid $win.list.hscroll                   -stick new
        
        #entry and buttons:
        
        ttk::frame $win.entry
        ttk::entry $win.entry.entry
        ttk::label $win.entry.label -text Value
        ttk::button $win.entry.add -text Add  -command [mymethod _addItem]
        ttk::button $win.entry.replace -text Replace -command [mymethod _replaceItem]
        
        grid $win.entry.entry $win.entry.label -sticky nsew
        grid $win.entry.add $win.entry.replace -sticky nsw
        
        grid $win.list
        grid $win.entry
        
        $self configurelist $args
        
        # context menu (right click on item).
        #  Allows repositioning and deletion:
        
        menu $win.context -tearoff 0
        $win.context add command -label Delete -command [mymethod _delete]
        $win.context add command -label {Move up} -command [mymethod _moveUp]
        $win.context add command -label {Move down} -command [mymethod _moveDown]
        
        bind  $win.list.listbox <Button-3> [mymethod _contextMenu %x %y %X %Y]
    }
    #--------------------------------------------------------------------------
    # Configuration handling:
    
    ##
    # _cfgValues
    #     New list of values
    # @param name -configuration option name.
    # @param value - New values.
    #
    method _cfgValues {name value} {
        set l $win.list.listbox
        $l delete 0 end
        foreach item $value {
            $l insert end $item
        }
    }
    ##
    # _cgetValues
    #    Return the list of values in the list box.
    #
    # @param name - configuration option name.
    #
    method _cgetValues {name} {
        
        return [$win.list.listbox get 0 end]
    }
    #-------------------------------------------------------------------------
    # Event handling.
    
    ##
    # _addItem
    #    Add an item from the text entry box to the end of the list.
    #
    method _addItem {} {
        set item [string trim [$win.entry.entry get]]
        if {$item ne ""} {
           $win.list.listbox insert end $item
           $win.entry.entry delete 0 end
        }
    }
    ##
    # _replaceItem
    #    Replace the selected item with the entry.
    #    - There must be a new value.
    #    - There must be a selection.
    #
    method _replaceItem {} {
        # Get the value it must be nonempty:
        
        set value [string trim [$win.entry.entry get]]
        if {$value eq ""} {
            tk_messageBox -icon info -type ok -title "No value" \
                -message {Need a value to replace} -parent $win
            return
        }
        #  Get the selection - it too must be non empty:
        
        set sel [$win.list.listbox curselection]
        if {$sel eq ""} {
             tk_messageBox  -icon info -type ok -title "No selection" \
                 -message {No item selected for replacement} \
                 -parent $win
                    
             return
        }
        $win.list.listbox delete $sel
        $win.list.listbox insert $sel $value
        
    }
    ##
    # _contextMenu
    #   Select the item under the mouse and provide a pop up menu to
    #   manipuate that item.
    #
    # @param x,y - window relative pointer coords.
    # @param X,Y - screen relativel pointer coords.
    #
    method _contextMenu {x y X Y} {
       set l $win.list.listbox
       set index [$l nearest $y]
       
       $l selection clear 0 end
       $l selection set $index $index
       
       $win.context post $X $Y 
    }
    ##
    # _delete
    #    Delete the selected item, if there is one.
    #
    method _delete {} {
        set l $win.list.listbox
        set sel [$l curselection]
        if {$sel ne ""} {
           $l delete $sel $sel
        }
    }
    ##
    # _moveUp
    #   Move the selected item up one slot in the list box.
    #
    method _moveUp {} {
       set l $win.list.listbox
       
       set sel [$l curselection]
       if {$sel ne "" && $sel > 0} {
          set text [$l get $sel $sel]
          $l delete $sel
          $l insert [incr sel -1] $text
          
       }
    }
    ##
    # _moveDown
    #     Move the selected item down one slot in the list box.
    #
    method _moveDown {} {
       set l $win.list.listbox
       
       set sel [$l curselection]
       set n   [$l size]
       if {$sel ne "" && $sel < ($n-1)} {
            set text [$l get $sel $sel]
            $l delete $sel
            $l insert [incr sel]  $text
       }
       
    }
    
}

 #==============================================================================
 #     The Main GUI has several components:
 
 ##
 # @class rdo::CommonAttributes
 #
 #  Form to allow the input of common attributes;
 #
 # *   - -type -  one of: {ddas, vmusb, ccusb, custom}
 # *   - -container  If not an empty string the container that will run the readout.
 # *   - -host    Host in which the Readout will run.
 # *   - -directory - working directory in which the Readout will run.
 # *   - -sourceid - Event builder source id.
 # *   - -ring  - output ring buffer.
 # *   - -service - REST service name used to control the Readout.
 #
 #  Other OPTIONS:
 #    -containers - containers to select between.
 #    -typeselectcommand - script when the radio buttons holding the Readout Type
 #        change.
 #
 snit::widgetadaptor rdo::CommonAttributes {
    option -type
    option -containers  -default [list] -configuremethod _configContainers;       # Containers to chose between
    option -container;        # Selected container.
    option -host
    option -directory
    option -sourceid -default 0
    option -ring -default $::tcl_platform(user)
    option -service -default ReadoutREST
    option -typeselectcommand -default [list]
    
    variable daqtypes [list XIA VMUSB CCUSB Custom]
    
    constructor args {
        installhull using ttk::frame
        
        #  The Readout types
        
        ttk::labelframe $win.types -text {Readout Type}
        set radios [list]
        foreach type $daqtypes {
            lappend radios [ttk::radiobutton $win.types.[string tolower $type] \
                -variable [myvar options(-type)] -value $type -text $type \
                -command [mymethod _dispatchType]]
        }
        set options(-type) [lindex $daqtypes 0]
        grid {*}$radios
        
        #  Container and host:
        
        ttk::labelframe $win.container -text {Container}
        ttk::combobox $win.container.container \
            -values [list] \
            -textvariable [myvar options(-container)]
        ttk::entry $win.container.host -textvariable [myvar options(-host)]
        ttk::label $win.container.hostlabel -text Host
        grid $win.container.container  $win.container.host $win.container.hostlabel
        
        # directory, sourceid
        
        ttk::labelframe $win.dir -text {Directory}
        ttk::entry $win.dir.dir -textvariable [myvar options(-directory)]
        ttk::button $win.dir.browse -text {Browse...} -command [mymethod _browsedir]
        grid $win.dir.dir $win.dir.browse -sticky ew
    
        ttk::labelframe $win.sid -text {Source Id}
        ttk::spinbox $win.sid.sid \
            -from 0 -to 100000 -increment 1 \
            -textvariable [myvar options(-sourceid)]
        ttk::label $win.sid.label -text {Source Id}
        grid $win.sid.sid $win.sid.label -sticky ew
        
        # ring and REST service.
        
        ttk::labelframe $win.ring -text {Output Ring}
        ttk::entry $win.ring.ring -textvariable [myvar options(-ring)]
        ttk::label $win.ring.label -text {Ring name}
        grid $win.ring.ring $win.ring.label -sticky ew
        
        ttk::labelframe $win.service -text {REST service name}
        ttk::entry $win.service.name -textvariable [myvar options(-service)]
        ttk::label $win.service.label -text {Service}
        grid $win.service.name $win.service.label -sticky ew
        
        #  Grid the top level frames:
        
        grid $win.types $win.container -sticky ew
        grid $win.dir $win.sid -sticky ew
        grid $win.ring $win.service -sticky ew
        
         
        $self configurelist $args 
        
    }
    #--------------------------------------------------------------------------
    #
    # Configuration methods
    
    
    ##
    # _configContainers
    #    Configure the containers in the $win.container.container combobox.
    #
    # @param name - name of configuration option
    # @param value - new value
    #
    method _configContainers {name value} {
        set options($name) $value
        $win.container.container configure -value $value
    }
    #-------------------------------------------------------------------------
    #  Event handling
    
    
    ##
    # _dispatchType
    #   Called when the type changes.
    #   The user script, if it exists is called with the new type as a parameter.
    #
    method _dispatchType {} {
        set userscript $options(-typeselectcommand)
        if {$userscript ne ""} {
            lappend userscript $options(-type)
            uplevel $userscript
        }
        
    }
    ##
    # _browsedir
    #   Browse for the current directory string.
    #
    method _browsedir {} {
        set dir [tk_chooseDirectory -parent $win -title {Choose directory}]
        
        # Only change if one was chosen:
        #
        if {$dir ne ""} {
            $self configure -directory $dir
        }
    }
 }
 ##
 # @class rdo::XIAAttributes
 #
 #   XIA readouts for version 11.4 experimental and later have
 #   The following attributes (class options).
 #
 #  -  -sorthost   - host in which the sorter runs.
 #  -  -sortring   - Ringbuffer that gets the sorted output.
 #  -  -sortwindow - Sorter sliding window.
 #  -  -fifothreshod - FIFO Threshold
 #  -  -buffersize  - Readout program's buffersize (clump).
 #  -  -infinityclock (bool true if infinity clock should be used).
 #  -  -clockmultiplier - Multiply external clock by this to get ns.
 #  -  -scalerperiod - Seconds between scaler readouts.
 #
 snit::widgetadaptor rdo::XIAAttributes {
    option -sorthost   -default [info hostname]
    option -sortring   -default $::tcl_platform(user)_sort
    option -sortwindow -default 5
    option -fifothreshold -default [expr 8192*10]
    option -buffersize    -default 16384
    option -infinityclock  -default 0
    option -clockmultiplier -default 1
    option -scalerperiod    -default 2
    
    constructor args {
        installhull using ttk::frame
        #  Title:
        
        ttk::label $win.title -text {XIA Readout Attributes}
        
        #  Sort host entry and label:
        
        ttk::labelframe $win.host -text {Sort host}
        ttk::entry $win.host.host -textvariable [myvar options(-sorthost)]
        ttk::label $win.host.label -text {Sort Host}
        grid $win.host.host $win.host.label -sticky ew
        
        # Sort ring
        
        ttk::labelframe $win.ring -text {Sort output ring}
        ttk::entry $win.ring.ring -textvariable [myvar options(-sortring)]
        ttk::label $win.ring.label -text {Output Ring}
        grid $win.ring.ring $win.ring.label -sticky ew
        
        #  sort window
        
        ttk::labelframe $win.sortwin -text {Sort window}
        ttk::spinbox $win.sortwin.window \
            -from 1 -to 1000 -increment 1 \
            -textvariable [myvar options(-sortwindow)]
        ttk::label $win.sortwin.label -text {Seconds}
        grid $win.sortwin.window $win.sortwin.label -sticky ew
        
        #  FIFO Threshold
        
        ttk::labelframe $win.fifo -text {Readout Threshold}
        ttk::spinbox $win.fifo.fifo \
            -from 1024 -to [expr 128*1024] -increment 512 \
            -textvariable [myvar options(-fifothreshold)]
        ttk::label $win.fifo.label -text {FIFO Threshold}
        grid $win.fifo.fifo $win.fifo.label -sticky ew
 
        # Buffer size
        
        ttk::labelframe $win.bsize -text {Buffer Size}
        ttk::spinbox    $win.bsize.bsize -from 8192 -to [expr 1024*1024] -increment 1024 \
            -textvariable [myvar options(-buffersize)]
        ttk::label $win.bsize.label -text {Readout Buffersize}
        grid $win.bsize.bsize $win.bsize.label -sticky ew
        
        #clock parameters
        
        ttk::labelframe $win.clock -text {Clock parameters}
        ttk::checkbutton $win.clock.infinity -text Infinity \
             -onvalue 1 -offvalue 0 -variable [myvar options(-infinityclock)]
        ttk::entry $win.clock.multiplier -textvariable [myvar options(-clockmultiplier)]
        ttk::label $win.clock.mlabel -text {Multiplier}
        grid $win.clock.infinity $win.clock.multiplier $win.clock.mlabel -sticky ew
        
        #scaler period:
        
        ttk::labelframe $win.scaler -text {Scalers}
        ttk::spinbox $win.scaler.period -from 2 -to 3600 -increment 1 \
            -textvariable [myvar options(-scalerperiod)]
        ttk::label $win.scaler.label -text {Readout period}
        
        grid $win.scaler.period $win.scaler.label -sticky ew
        
        grid $win.title -sticky w
        grid $win.host $win.ring -sticky ew
        grid $win.sortwin $win.fifo -sticky ew
        grid $win.bsize $win.clock -sticky ew
        grid $win.scaler -columnspan 2 -sticky ew
        
        
        $self configurelist $args
    }
     
 }
##
# @class XXUSBAttributes
#   Prompting form for attributes of an XXUSBReadout.  For this we need:
#
#  -  -byserial  - True if we want a specific serial number
#  -  -serialstring - Serial number string.
#  -  -daqconfig   - DAQ configuration file.
#  -  -usectlserver - Enable control configuration
#  -  -ctlconfig   - Control config file.
#  -  -port        - ctlconfig port.
#  -  -enablelogging - True to turn on logging.
#  -  -logfile     - Log file name.
#  -  -logverbosity - Verbosity of logfile.
#
snit::widgetadaptor rdo::XXUSBAttributes {
    option -byserial -default 0
    option -serialstring
    
    option -daqconfig
    
    option -usectlserver -default 0 -configuremethod _setCtlServerVisibility
    option -ctlconfig
    option -port -default 1024
    
    option -enablelogging -default 0 -configuremethod _setLoggingVisibility
    option -logfile
    option -logverbosity -default 0
    
    constructor args {
        installhull using ttk::frame
        ttk::label $win.title -text {XXUSBReadout attributes}
        
        # DAQ Config file.
        
        ttk::labelframe $win.daqconfig -text {DAQ configuration}
        ttk::entry $win.daqconfig.config -textvariable [myvar options(-daqconfig)]
        ttk::button $win.daqconfig.browse -text {Browse...} -command [mymethod _browseDAQFile]
        grid $win.daqconfig.config $win.daqconfig.browse -sticky ew
        
        # Control configuration :
        
        ttk::labelframe $win.ctlconfig -text {Control Configuration}
        ttk::checkbutton $win.ctlconfig.enable -text {Use Control Server} \
            -onvalue 1 -offvalue 0 -variable [myvar options(-usectlserver)] \
            -command [mymethod _controlOptions]
        ttk::entry $win.ctlconfig.ctlconfig \
            -textvariable [myvar options(-ctlconfig)]
        ttk::label $win.ctlconfig.filelbl -text {Config File}
        ttk::button $win.ctlconfig.browse \
            -text Browse... -command [mymethod _browseCtlConfig]
        ttk::spinbox $win.ctlconfig.port -textvariable [myvar options(-port)] \
            -from 1024 -to 65535 -increment 1
        ttk::label $win.ctlconfig.plabel -text {Server Port}
        grid $win.ctlconfig.enable -sticky w
        grid $win.ctlconfig.ctlconfig $win.ctlconfig.filelbl \
             $win.ctlconfig.browse $win.ctlconfig.port $win.ctlconfig.plabel \
             -sticky ew
        
        # Logging configuration:
        
        ttk::labelframe $win.logging -text {Logging configuration}
        ttk::checkbutton $win.logging.enable -text Enable -onvalue 1 -offvalue 0 \
             -variable [myvar options(-enablelogging)] \
             -command [mymethod _loggingOptions]
        ttk::entry $win.logging.file -textvariable [myvar options(-logfile)]
        ttk::label $win.logging.label -text {Log File}
        ttk::button $win.logging.browse -text {Browse...} \
             -command [mymethod _browseLogfile]
        ttk::spinbox $win.logging.level -textvariable [myvar options(-logverbosity)] \
         -from 0 -to 3 -increment 1
        ttk::label $win.logging.levellabel -text {Verbosity level}
        grid $win.logging.enable -sticky w
        grid $win.logging.file $win.logging.label $win.logging.browse -sticky ew
        grid $win.logging.level $win.logging.levellabel -sticky w
        
        grid $win.title     -sticky w
        grid $win.daqconfig -sticky ew
        grid $win.ctlconfig -sticky ew
        grid $win.logging  -sticky ew
        
        $self configurelist $args
        $self _controlOptions
        $self _loggingOptions
    }
    #---------------------------------------------------------------------------
    # Configuration handling.
    
    
    
    ##
    #  _setCtlServerVisibility
    #    Configure the flag that uses/does not use control server,
    #    just set the options array and invoke _controlOptions.
    #
    method _setCtlServerVisibility {name value} {
        set options($name) $value
        
        $self _controlOptions
    }
    ##
    # _setLoggingVisibility
    #    Configure the flag that enables/disables logging. This enables
    #    disables some widgets too
    #
    method _setLoggingVisibility {name value} {
        set options($name) $value
        $self _loggingOptions
    }
    #---------------------------------------------------------------------------
    # Event handlers:
    
    ##
    #  _browseDAQfile
    #    Select a daqconfig file:
    #
    method _browseDAQFile {} {
        $self _browseForOption -daqconfig
    }
    ##
    # _browseCtlConfig
    #   Browse for the control config file
    #
    method _browseCtlConfig {} {
        $self _browseForOption -ctlconfig
    }
    ##
    # _browseLogfile
    #    Browse for a logfile.
    #
    method _browseLogfile {} {
         $self _browseForOption -logfile
    }
        
    
    
    ##
    # _controlOptions
    #   Called when options(-usectlserver) changes
    #
    method _controlOptions {} {
        if {$options(-usectlserver)} {
            set state normal
        } else {
            set state disable
        }
        foreach subwidget [list ctlconfig browse port plabel] {
            $win.ctlconfig.$subwidget config -state $state
        }
            
        
    }
    ##
    # _loggingOptions
    #    Enable/disable logging option inputs.
    #
    method _loggingOptions {} {
        if {$options(-enablelogging)} {
            set state normal
        } else {
            set state disabled
        }
        $win.logging.file configure -state $state
        $win.logging.browse configure -state $state
        $win.logging.level configure -state $state
    }
    #---------------------------------------------------------------------------
    # Utils
    
    method _browseForOption {name} {
       set file [tk_getOpenFile -parent $win -title "DAQ config file" \
            -filetypes [list                                           \
                    {{Tcl Scripts} .tcl}                               \
                    {{All Files}    *}                                  \
        ]]
        if {$file ne ""} {
            $self configure $name $file
        }
    }
}

##
# CustomAttributes
#    The custom attributes that are required are:
#
#  -program - the program that will be run.
#
#  -options - option value pairs for command line options.
#  -environment - Environment variables and their values.
#  -parameters - Parameter following options on the command line.
#
#  These are presented as listboxes that can be edited.
#
#
snit::widgetadaptor rdo::CustomAttributes {
    option -program -default ""
    option -options -default [list]
    option -environment -default [list]
    option -parameters  -default [list]

    constructor args {
        installhull using ttk::frame
        
        ttk::labelframe $win.program -text {Readout Program}
        ttk::entry      $win.program.entry -textvariable [myvar options(-program)]
        ttk::button     $win.program.browse -text {Browse...}
        
        grid $win.program.entry $win.program.browse -sticky ew
        
        ttk::labelframe $win.parameters -text {Parameters and environment}
        ttk::label      $win.parameters.note \
            -text {NOTE: General options will set options and environment variables as well} \
            -foreground DarkOrange3
        
        grid $win.parameters.note
        
        grid $win.program -sticky ew
        grid $win.parameters -sticky ew
        
        $self configurelist $args
    }
} 

    


