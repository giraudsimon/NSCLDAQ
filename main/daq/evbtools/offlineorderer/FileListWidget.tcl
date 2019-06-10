#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#      NSCLDAQ Development Group
#
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

# This software is Copyright by the Board of Trustees of Michigan
# State University (c) Copyright 2297884. 

# @file  FileListWidget.tcl 
# @author Jeromy Tompkins 


package provide FileListWidget 1.0

package require snit
package require Tk


## @brief A simple widget that allows the user to select files and populate a 
#   list
#
# It is implemented as a simple widget without the MVP paradigm. This was
# a choice mainly to just keep this as a simple widget that is embodied 
# in a single class.
#
snit::widget FileList {

  option -filelist  -default "" ; #< the list of files being managed
  option -sort      -default 1  ; #< whether or not to keep the list sorted
  

  variable m_nAlreadyAdded 0    ; #< a simple counter to ensure unique ids

  ## @brief Build the GUI
  #
  constructor {args} {
    $self configurelist $args
    $self buildGUI 

  }  

  ## @brief Assembles the widgets into a megawidget
  #
  method buildGUI {} {
      ttk::treeview $win.list -show tree 
      $win.list column #0 -stretch on -minwidth 10
      ttk::scrollbar $win.xscroll -orient horizontal -command "$win.list xview"
      $win.list configure -xscrollcommand "$win.xscroll set"

      ttk::button   $win.addrun      -text "Add Files"    -command [mymethod onAddFiles]
      ttk::button   $win.rem      -text "Remove Selected" -command [mymethod onRemove]

      grid $win.list              -  -sticky nsew
      grid $win.addrun $win.rem    -sticky ew
      grid rowconfigure $win    {0 1} -weight 1
      grid columnconfigure $win {0 1} -weight 1 -minsize 100
  }


  ## @brief Handle presses of the "Add Files" button
  #
  # This launches a file selector dialogue and then adds the 
  # selection to the list of files. Optionally it will sort the 
  # list of files if options(-sort) is 1.
  method onAddFiles {} {

    # choose to view evt files by default but allow to view others
    set types {
                {{Evt Files} {.evt}}
                {{All Files} *     }
              } 

    # Request the names of files
    set selection [tk_getOpenFile -multiple true -filetypes $types ]

    # optionally sort the selected entries
    if {$options(-sort)} {
      # Make sure that these are in increasing order
      set selection [lsort -increasing $selection]
    }

    # add the new files to the end of the list 
    $self appendFiles $selection

    # sort the entire list of entries
    if {$options(-sort)} {
      $self sortList
    }
  }

  ## @brief Inserts a list of files into the treeview widget 
  #
  # This will insert the files as new items to the end of the
  # the root node in the treeview. Each entry in the list will
  # receive a unique id. Only the tail will be displayed while the
  # value associated with it will be the absolute path.
  #
  method appendFiles {selection} {
    # insert the new files from the selection
    foreach f $selection {
      $win.list insert {} end -id file$m_nAlreadyAdded \
                              -text [file tail $f] -value $f
      incr m_nAlreadyAdded 
    }
  }

  ## @brief Reorder the displayed items in ascending order
  #
  method sortList {} {
    # order them
    # get the current list of children
    set children [$win.list children {}]

    # remove them from view but don't delete them
    $win.list detach $children

    # create a dict of (file name:id) pairs
    set map [dict create]
    foreach child $children {
      dict set map [$win.list item $child -text] $child
    }

    # sort the keys of that dict in ascending order
    set fnames [lsort -increasing [dict keys $map]]

    # put the sorted values back into the list in the 
    # new ordering
    foreach fname $fnames {
      $win.list move [dict get $map $fname] {} end
    }
  }

  ## @brief Remove the selected entries from the treeview
  # 
  # Note that removing files will not change the order
  # of the residual items. If they were ordered prior
  # to the deletion, then they will remain ordered after
  # it.
  #
  method onRemove {} {

    set entries [$win.list selection]
    if {[llength $entries] == 0 } {
      tk_messageBox -icon error -message "User must select files to remove from list"
    }
    $win.list delete $entries
  }

  ## Delete all of the children of the root node
  # 
  #
  method clearTree {} {
    set nodes [$win.list children {}]
    $win.list delete $nodes
  }

  ## @brief Get the list of absolute paths in the treeview
  # 
  # This simply builds a list of the absolute paths held
  # by the treeview object. Note that the displayed values
  # are actually just the tail of the file name and that 
  # the -value option of the treeview items contains the 
  # absolute path. The order of the treeview widget will be
  # maintained.
  #
  # @returns a list of the absolute paths 
  #
  method getFiles {} {

    set files [list]

    set children [$win.list children {}]
    foreach child $children {

      set fname [$win.list item $child -value]
      lappend files $fname
    }

    return $files
  }


}
