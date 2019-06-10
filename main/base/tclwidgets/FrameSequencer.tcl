
package provide FrameSequencer 1.0

package require FrameManager
package require snit
package require Tk

snit::widget FrameSequencer {

  variable m_frameTree

  component m_manager


  constructor {args} {
    set m_frameTree [dict create]

    $self buildGui
  }

  method buildGui {} {

    ttk::frame $win.buttons
    ttk::button $win.buttons.back -text "Back" -command [mymethod onBack] -style "Flat.TButton"
    ttk::button $win.buttons.next -text "Next" -command [mymethod onNext] -style "Flat.TButton"
    grid $win.buttons.back -row 0 -column 0 -sticky w
    grid $win.buttons.next -row 0 -column 1 -sticky e
    grid rowconfigure    $win.buttons 0 -weight 1
    grid columnconfigure $win.buttons 0 -weight 1

    install m_manager using FrameManager $win.content
   
    grid $win.content -sticky nsew 
    grid $win.buttons -sticky new

    grid rowconfigure    $win {0 1} -weight 1
    grid rowconfigure    $win 0 -minsize 300
    grid columnconfigure $win 0 -weight 1 -minsize 300
  }

  method staticAdd {name fr parent {onviewscript ""}} {
    # add the frame to the frame manager
    $m_manager add $name $fr $onviewscript

    # create an entry in the tree 
    dict set m_frameTree $name [dict create back {} next {}]
    dict set m_frameTree $name back $parent
  }

  method dynamicAdd {name fr {onviewscript ""}} {
    set prevFrame [$m_manager getCurrent]

    $self staticAdd $name $fr $prevFrame $onviewscript 

    $self select $name
  }


  method select {name} {
    $m_manager select $name

    set parent [dict get $m_frameTree $name back]

    if {$parent ne {}} {
      grid $win.buttons.back -sticky nw
    } else {
      grid remove $win.buttons.back
    }

    set child [dict get $m_frameTree $name next] 
    if {$child ne {}} {
      grid $win.buttons.next -sticky e
    } else {
      grid remove $win.buttons.next
    }

  }


  method onBack {} {

    set current [$m_manager getCurrent]
    set parent [dict get $m_frameTree $current back]
    
    if {$parent ne {}} {
      $self select $parent
    }
  }

  method onNext {} {

    set current [$m_manager getCurrent]
    set child [dict get $m_frameTree $current next]
    
    if {$child ne {}} {
      $self select $child
    }
  }

  method getTree {} {
    return $m_frameTree
  }
  
}

