
package provide TransientLabel 1.0

package require snit
package require Tk

## @brief A simple widget that schedules the clearing of the -text after 
#         a fixed amount of elapsed time
#
#
# This is really just a ttk::label with a specialized -text option. When the
# user configures the text, it will show up as they specified but will clear
# after a certain period of time. This amount of time is set via the
# -timerlength option, which takes a value of milliseconds.
#
snit::widgetadaptor TransientLabel {

  option -defaultstring -default {} ;# the value -text falls back to
  option -timerlength -default 2000 ;# number of milliseconds to display -text
  option -text -default {} -configuremethod SetText ;# specialized -text option

  # delegate just about everything to the hull so this behaves as any other
  # ttk::lable object.
  delegate option * to hull
  delegate method * to hull

  variable _lastOpId -1 ;# the id scheduled by the last after cmd

  ## @brief Construct the hull and parse options
  #
  # @param args   option-value pairs
  #
  constructor {args} {
    installhull using ttk::label
    $self configurelist $args
  }

  destructor {
    after cancel $_lastOpId
  }

  ## @brief Callback for the -text option
  #
  # Sets the value of the -text option, schedules the fallback to the default
  # string, and makes sure that any previous events scheduled are cleared. This
  # last activity ensures that the most recent value of -text gets it full time
  # displayed and is not cleared prematurely.
  #
  # @param opt    name of option (-text)
  # @param msg    message
  #
  method SetText {opt msg} {
    if {$_lastOpId != -1} {
      after cancel $_lastOpId
    }

    # set the option 
    $hull configure -text $msg
    set options($opt) $msg

    # schedule the clear event
    set _lastOpId [after [$self cget -timerlength] \
      [list $self configure -text [$self cget -defaultstring]]]
  }

} ;# end of TransientLabel snit::widgetadaptor


