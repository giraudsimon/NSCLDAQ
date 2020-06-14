#/*
#*-------------------------------------------------------------
# 
# CAEN SpA 
# Via Vetraia, 11 - 55049 - Viareggio ITALY
# +390594388398 - www.caen.it
#
#------------------------------------------------------------
#
#**************************************************************************
#* @note TERMS OF USE:
#* This program is free software; you can redistribute it and/or modify it under
#* the terms of the GNU General Public License as published by the Free Software
#* Foundation. This program is distributed in the hope that it will be useful, 
#* but WITHOUT ANY WARRANTY; without even the implied warranty of 
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
#* software, documentation and results solely at his own risk.
#*
#* @file      tweaker.tcl
#* @brief     Graphically tweak the parameters of a COMPASS XML file.
#* @author   Ron Fox
#*
#*/

##
# tweaker.tcl
#    This Tcl script allows some of the values for digitizers in the COMPASS
#    configuration file to be tweaked.  We envision the following minimally
#    for all digitizer types:
#
#    -  DC offset.
#    -  Trigger threshold.
#    -  Pre-trigger
#  Additional digitizer specific values may be modified as well.
#

#  Information about the program:

set programAuthors {Ron Fox}
set programVersion {v1.0-000}
set programReleaseDate {June 13, 2020}

package require Tk
package require tdom
package require snit

#----------------------- COMPASSdom class --------------------------
##
# COMPASSdom
#   This class offers high level access to a DOM for a COMPASS XML file.
# Construction:
#   COMPASSdom name|%AUTO% filename.
#
# Mehods:
#   getBoards           - Returns a list of tokens that can be used
#                         to access each board.
#   getBoardDescription - Get a description of a board given a token
#                         from getBoards.
#
snit::type COMPASSdom {
    variable dom;         # Tcl DOM
    variable root;        # document root element handle.
    
    
    constructor {args} {
        if {[llength $args] != 1} {
            error "COMPASSdom consructor requires a document file"
        }
        set filename [lindex $args 0]
        set fd [open $filename r]
        set xml [read $fd]
        close $fd
        set dom [dom parse $xml]
        
        #  The only verification we can make at at this time
        #  is that the dom root element is a
        #  <configuration> tag.
        
        set root [$dom documentElement]
        set tag [$root nodeName]
        if {$tag ne "configuration"} {
            error "$filename does not contain a COMPASS configuration file"
        }
    }
    #----------------------------- public methods -----------------------------
    ##
    # getBoards
    #    Gets handles to each board in the system.
    #
    # @return - list of board handles.
    # @note   - <board> tags are immediate children of the root (<configuration>)
    #           node.  We make no assumptions about whether or not there are
    #           other non <board> children:
    #
    method getBoards {} {
        set children [$root childNodes];    # Operations on the board affect underlying dom.
        set result [list]
        foreach child $children {
            if {[$child nodeName] eq "board"} {
                lappend result $child
            }
        }
        return $result
    }
    ##
    # getBoardDescription
    #   Returns a description of a board (e.g. one returned from getBoards)
    #  @param board - a board id e.g. an element of the list returned by getBoards.
    #  @return dict - with the following keys:
    #                 model  - model name of the digitizer.
    #                 serial - model serial number
    #                 dpptype - Type of dpp  (PHA or PSD or waveform).
    #                 connecton - OPTICAL or USB
    #                 link      - Connection link number.
    #                 node      - CONET node number.
    #                 address   - VME base address.
    #        
    #
    method getBoardDescription {board} {
        set result [dict create]
        set children [$board childNodes]
        foreach child $children {
            set tag [$child nodeName]
            
            #  Only do the next bit if the tag is interesting to us:
            
            if {$tag in [list modelName serialNumber linkNum connectionType amcFirmware address conetNode]} {
                set value [[$child firstChild] nodeValue]
                switch  -exact -- $tag {
                    modelName {
                        dict set result model $value
                    }
                    serialNumber {
                        dict set result serial $value
                    }
                    amcFirmware {
                        set dppType [$self _computeDppType $child]
                        dict set result dpptype $dppType
                    }
                    connectionType {
                        dict set result connection $value
                    }
                    linkNum {
                        dict set result link $value
                    }
                    conetNode {
                        dict set result node $value
                    }
                    address {
                        dict set result address $value
                    }
                }
            }
        }
        
        return $result
    }
    #--------------------------- private methods ------------------------------
    ##
    # _computeDppType
    #   Given the <amcFirmware> tag node, determines the firmware type.
    #   - Get the firmware major node.
    #   - if the value of that node is 139 -> PHA
    #   - if the value of that node is 136 -> PSD
    #   - Otherwise, at least for now, waveform.
    #
    # @param amcfw  - Amc firmware node tag.
    # @result the firmware type string
    #
    method _computeDppType {amcfw} {
        set major [$amcfw getElementsByTagName major]
        if {[llength $major] == 0} {
            error "<amcFirmware> tag does not have a <major> tag"
        }
        set result [[$major firstChild] nodeValue]
        if {$result eq 136} {
            set result PSD
        } elseif {$result eq 139} {
            set result PHA
        } else {
            set result waveform
        }
        
        return $result
    }
}
    

    



#----------------------- titleManager class ----------------------
##
# titleManager
#   Manages the title of a top level window. The title will be of the form:
#     application-name ?- filename ?[modified]??
#
#  Options:
#    -appname - (readonly) name of the application leftmost part of the file.
#    -widget  - (readonly) widget path of the top level (must be a toplevel.)
# Methods
#   openFile   - Add a filename to the title.
#   modifyFile - Add [modified] to the title.
#   saveFile   - Remove [modified] from the title if present.
#   closeFile  - Remove the filename (and [modified if present]) from the title.
#
snit::type titleManager {
    option -appname -readonly
    option -widget -readonly
    
    variable  filename  "";      # non empty if file is open.
    variable  isModified 0;      # boolean true if the file is modified..
    
    constructor args {
        $self configurelist $args
        
        $self _updateTitle
    }
    #---------------------- public methods -----------------------
    ##
    # openFile
    #   - set the new filename.
    #   - clear the isModified flag.
    #   - update the title.
    #
    # @param newFile - filename being opened.
    # #
    method openFile {newFile} {
        set filename $newFile
        set isModified 0
        $self _updateTitle
    }
    ##
    # modifyFile
    #   Set the modify flag. This is not legal when there is no file.
    #
    method modifyFile {} {
        if {$filename eq ""} {
            error "Trying to set the modify flag but there's no file open"
        }
        set isModified 1
        $self _updateTitle
    }
    ##
    # saveFile
    #   clear the modify flag. This is not legal when there's no
    #   file defined.
    #
    method saveFile {} {
        if {$filename eq ""} {_
            error "There is no file to save!"
        }
        set isModified 0
        $self _updateTitle
    }
    ##
    # closeFile
    #   - clear the filename.
    #   - clear the modified flag.
    #   - update the title.
    #
    #  not legal if there's no file to close.
    #
    method closeFile {} {
        if {$filename eq ""} {
            error "There is no file to close"
        }
        set filename ""
        set isModified 0
        $self _updateTitle
    }
    #---------------------- private methods --------------------
    ##
    # _updateTitle
    #   Compute the title and set it in the window title.
    #
    method _updateTitle {} {
        set title $options(-appname)
        if {$filename ne ""} {
            append title " $filename"
            if {$isModified} {
                append  title {  [modified]}
            }
        }
        wm title $options(-widget) $title
    }
}

#------------------------------------------------------------------------------
##
# help->About
#   Pop up the help about dialog.
#
proc help->About {} {
    set msg "tweaker version $::programVersion\n"
    append msg "Graphically edit CAEN Compass Configuration files\n"
    append msg "Author  : $::programAuthors\n"
    append msg "Released: $::programReleaseDate"
    
    tk_messageBox -icon info -type ok -title "Program information" -message $msg
}
##
# createAppMenu
#    Create the application menu. This consists of the following:
#
#  File menu:
#    -  Open    - Open a COMPASS configuration file
#    -  Save    - Save current configuration file back to original file.
#    -  Save As.. - Save current configuration file to a different file.
#    -  Exit    - Exit the document.  If modified prompt to save.
#    -  Quit    - Exit unconditionally without propmting.
#
# Help menu:
#    About - Current version of help menu.
#
proc createAppMenu {} {
    menu .appmenu

    menu .appmenu.filemenu -tearoff 0
    .appmenu add cascade -label File -menu .appmenu.filemenu
    .appmenu.filemenu add command -label Open...
    .appmenu.filemenu add separator
    .appmenu.filemenu add command -label Save
    .appmenu.filemenu add command -label {Save As...}
    .appmenu.filemenu add separator
    .appmenu.filemenu add command -label Exit
    .appmenu.filemenu add command -label Quit
    
    
    menu .appmenu.helpmenu -tearoff 0
    .appmenu add cascade -label Help -menu .appmenu.helpmenu
    .appmenu.helpmenu add command -label About... -command help->About
    
    . configure -menu .appmenu
}

    

    

    

    
