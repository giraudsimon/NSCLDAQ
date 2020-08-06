
package provide DefaultActions 1.0
package require ReadoutGUIPanel

namespace eval DefaultActions {
  variable name ""

  proc handleError str { 
    variable name
    set trimmedstr [string trimright $str " \n"]
    ReadoutGUIPanel::Log $name error $trimmedstr
  }

  proc handleLog {str} {
    variable name
    set trimmedstr [string trimright $str " \n"]
    ReadoutGUIPanel::Log $name log $trimmedstr
  }

  proc handleWarning {str} {
    variable name
    set trimmedstr [string trimright $str " \n"]
    ReadoutGUIPanel::Log $name warning $trimmedstr
  }

  proc handleDebug {str} {
    variable name
    set trimmedstr [string trimright $str " \n"]
    ReadoutGUIPanel::Log $name debug $trimmedstr
  }

  proc handleOutput {str} {
    variable name
    set trimmedstr [string trimright $str " \n"]
    ReadoutGUIPanel::Log $name output $trimmedstr
  }

  proc handleTclCommand {str} {
    set trimmedstr [string trimright $str " \n"]
    uplevel #0 eval $trimmedstr
  }

  namespace export handleError handleLog \
            handleWarning handleDebug \
            handleOutput handleTclCommand
  namespace ensemble create
}

