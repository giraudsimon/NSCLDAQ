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


package provide FrameManager 1.0

package require snit
package require Tk

## @brief A snit:;type to easily switch back and forth between visible frames
# 
snit::widget FrameManager {

  variable m_frameList ;# the list of frames
  variable m_current   ;# current frame name

  option -width  -default ""
  option -height -default ""

  ##
  #
  constructor {args} {
    $self configurelist $args

    set m_frameList [dict create]
    set m_current ""

    $self buildGui
  }

  ##
  #
  method buildGui {} {
    if {$options(-height) ne ""} {
      grid rowconfigure    $win 0 -weight 1 -minsize $options(-height)
    } else {
      grid rowconfigure    $win 0 -weight 1
    }
    if {$options(-width) ne ""} {
      grid columnconfigure $win 0 -weight 1 -minsize $options(-width) 
    } else {
      grid columnconfigure $win 0 -weight 1
    }
  }

  ## @brief Add a new frame
  #
  method add {name fr {onviewscript ""}} {
    set prevCurrent $m_current
    if {[dict size $m_frameList]==0} {
      set m_current $name
    }
    dict set m_frameList $name [dict create widget $fr script $onviewscript]
  }

  ##
  #
  method select {name} {
    set keys [dict keys $m_frameList]
    set m_current $name

    set index [lsearch $keys $name]
    if {$index >= 0} {
      set slaves [grid slaves $win]
      if {[llength $slaves]>0} {
        grid remove $slaves
      }

      set key [lindex $keys $index]
      set widget [dict get $m_frameList $key widget]

      grid $widget -in $win -sticky nsew

      grid rowconfigure    $win 0 -weight 1
      grid columnconfigure $win 0 -weight 1

      set script [dict get $m_frameList $key script]
      if {$script ne ""} {
        uplevel #0 eval [list $script]
      }
    }
  } 

  method getCurrent {} {
    return $m_current
  }

} ;# end of snit::type

