

package provide RunStateObserver 1.0

package require snit
package require TclRingBuffer
package require ring
package require Thread


snit::type RunStateObserver {

  option -ringurl -default {} -configuremethod onRingURL

  option -onbegin -default {}
  option -onpause -default {}
  option -onresume -default {}
  option -onend   -default {}

  variable acqThread {}

  constructor {args} {
    $self configurelist $args

    tsv::object $self continue
  }

  destructor {
    $self detachFromRing
  }

  method onRingURL {opt val} {
    # I shamelessly stole this nice regular expression recipe for matching
    # URLs from http://www.beedub.com/book/2nd/regexp.doc.html.
    set uriPattern {([^:]+)://([^:/]+)(:([0-9]+))?(/.*)}
    if {![regexp $uriPattern $val match protocol server x port path]} {
      return -code error "Invalid URI passed to RunStateObserver -ringurl option. Check URI syntax."
    }
    # drop the leading backslash from the path
    set path [string range $path 1 end]

    set options($opt) $val
    
  }

  method attachToRing {} {
    if {$acqThread eq {}} {
      set acqThread [$self startAcqThread ]
    } else {
      return -code error {Thread is already running. Cannot create second thread for this instance.}
    }
  }

  method detachFromRing {} {
    if {$acqThread ne {}} {
      tsv::set $self continue 0
      set acqThread {}
    }

  }

  method startAcqThread {} {
    set acqThread [thread::create]
    if {[thread::send $acqThread [list set ::auto_path $::auto_path] result]} {
        return -code error "Could not extend thread's auto_path"
    }
    if {[thread::send $acqThread [list package require TclRingBuffer] result]} {
        return -code error "Could not load RingBuffer package in acqthread: $result"
    }
    
    if {[thread::send $acqThread [list ring attach $options(-ringurl)] result]} {
        return -code error  "Could not attach to ring buffer in acqthread $result"
    }

    if {[thread::send $acqThread [list set instance $self] result]} {
        return -code error "Could not send instance name to acqthread $result"
    }

    #  The main loop will forward data to our handleData item.
    
    set myThread [thread::id]
    set getItems "proc getItems {obj tid uri} { 
        while {\[tsv::get \$::instance continue\]} {                                             
            set ringItem \[ring get \$uri {1 2 3 4}]             
            thread::send \$tid \[list \$obj handleData \$ringItem]     
        }                                                     
    }                                                         
    getItems $self $myThread $options(-ringurl)

    ring detach $options(-ringurl)
    thread::release
    "
    tsv::set $self continue 1
    thread::send -async $acqThread $getItems

    
    return $acqThread
  }

  method handleData {item} {
    
    switch [dict get $item type] {
      "Begin Run"  { 
        if {$options(-onbegin) ne {} } {
            uplevel #0 $options(-onbegin) [list $item]
        }
      }
      "End Run"  { 
        if {$options(-onend) ne {} } {
          uplevel #0 $options(-onend) [list $item]
        }
      }
      "Pause Run"  { 
        if {$options(-onpause) ne {} } {
          uplevel #0 $options(-onpause) [list $item]
        }
      }
      "Resume Run"  { 
        if {$options(-onresume) ne {} } {
          uplevel #0 $options(-onresume) [list $item]
        }
      } 
      default { puts [dict get $item type] }
    }
  }




}
