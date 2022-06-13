
package provide mscf16statesaver 1.0

package require snit
package require mscf16commandlogger
package require mscf16scriptheadergenerator

snit::type MSCF16StateSaver {

  variable _opts
  variable _presenter
  component _headerGenerator

  constructor {opts presenter} {
    set _opts $opts
    set _presenter $presenter
    install _headerGenerator using MSCF16ScriptHeaderGenerator %AUTO% $_opts
  }

  destructor {
    catch {$_headerGenerator destroy}
  }

  method SaveState {path} {
    if {[catch {open $path w} logFile]} {
      return -code error "Failed to open file $path"
    }

    # create the header lines that construct the device
    set headerLines [$_headerGenerator generateHeader]
    foreach line $headerLines {
      chan puts $logFile $line
    }

    $self SaveDeviceState $logFile
    $self SaveNames $logFile

    # done...
    close $logFile
  }

  method SaveDeviceState {logFile} {
    set logger [MSCF16CommandLogger %AUTO% $logFile]

    # typically registering a new handle will cause the presenter to
    # automatically update the view to the state of the device
    # We merely want to record the state of the view to a file so we 
    # store the previous state and then restore it.
    set oldUpdateState [$_presenter cget -autoupdate]
    set oldHandle [$_presenter cget -handle]

    # turn of autoupdate so we don't try to update
    $_presenter configure -autoupdate 0

    # insert the logging handle, then commit state of view to it.
    # there is no update that needs to happen.
    $_presenter configure -handle $logger
    $_presenter CommitViewToModel

    # the state of the device never changed, so reestablish the 
    # old handle without updating from the device.
    $_presenter configure -handle $oldHandle

    # turn of autoupdate so we don't try to update
    $_presenter configure -autoupdate 1 
    $logger destroy
  }

  method SaveNames {logFile} {
    chan puts $logFile "namespace eval ::MSCF16ChannelNames {}"
    for {set ch 0} {$ch<16} {incr ch} {
      chan puts $logFile "set ::MSCF16ChannelNames::chan$ch [set ::MSCF16ChannelNames::chan$ch]"
    }
  }
}
