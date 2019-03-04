#===================================================================
##
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2015.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#      NSCL DAQ Development Group 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
# @file   ACrdcXLM72.tcl
# @author Daniel Bazin and Jeromy Tompkins
# @note   This is a modified version of the original AGD16XLM72 class
#         that was written by Daniel Bazin. It has been updated to use
#         the cvmusb::CVMUSB and cvmusbreadoutlist::CVMUSBReadout methods
#         and to have a format that is more in line with other NSCLDAQ 
#         code.

package provide gd16xlm72 1.0

package require Itcl
package require xlm72
package require snit

itcl::class AGD16XLM72 {
	inherit AXLM72

	constructor {de sl} {
		AXLM72::constructor $de $sl
	} {}

	public method WriteDelayWidth {ch de wi}
	public method ReadDelayWidth {ch}

	public method WriteBypass {by} 
        public method ReadBypass {}

	public method WriteInspect {in}
	public method ReadInspect {}

        public method ReadFirmware {}

	public method Init {filename aname}
}

itcl::body AGD16XLM72::WriteDelayWidth {ch de wi} {
  set offset [expr $ch*4] 
  return [Write fpga $offset [expr $de+($wi<<8)]]
}

itcl::body AGD16XLM72::ReadDelayWidth {ch} {
  set offset [expr $ch*4] 

  set retValue [Read fpga $offset]


  # parse the results
  set delay [ expr 0xff & $retValue ]
  set width [ expr ($retValue>>8) & 0xff ]
  
  set result [list $delay $width]

  return $result
}

itcl::body AGD16XLM72::WriteBypass {by} {
  return [Write fpga 68 $by]
}

itcl::body AGD16XLM72::ReadBypass {} {
  return [Read fpga 68]
}

itcl::body AGD16XLM72::WriteInspect {in} {
  return [Write fpga 72 $in]
}

itcl::body AGD16XLM72::ReadInspect {} {
  return [Read fpga 72]
}

itcl::body AGD16XLM72::ReadFirmware {} {
  set fw [Read fpga 0]
  return $fw
}

itcl::body AGD16XLM72::Init {filename aname} {
  if {![file exists $filename]} {
    set msg "AGD16XLM72::Init initialization error. "
    append msg "File ($filename) does not exist."
    return -code error $msg
  } 

  # if we made it here, then the file exists
	source $filename

  if {![array exists $aname]} {
    set msg "AGD16XLM72::Init initialization error. "
    append msg "Array named \"$aname\" does not exist."
    return -code error $msg
  }
	AccessBus 0x10000
	for {set i 1} {$i <= 16} {incr i} {
    set delay [lindex [array get $aname delay$i] 1] 
    set width [lindex [array get $aname width$i] 1]
		WriteDelayWidth $i $delay $width
	}
	WriteBypass [lindex [array get $aname bypass] 1]
	WriteInspect [lindex [array get $aname inspect] 1]
	ReleaseBus
}


###############################################################################
###############################################################################
###############################################################################

## \brief Slow controls module for GD16XLM72
#
#  The AGD16XLM72Control module is used as the server-side component for 
#  talking to a GD16XLM72 via the slow controls server. It is intended to be
#  communicated with by the XLM72GateDelayGUI but can be used with any
#  application that speaks the proper protocol. It merely is a wrapper around 
#  an AGD16XLM72 driver for which it translates requests into actual low-level
#  driver calls.
snit::type AGD16XLM72Control {

  option -slot -default 0

  variable driver {}

  constructor args {
    $self configurelist $args
    set driver [AGD16XLM72 #auto {} $options(-slot)]
  }


  method Initialize driverPtr {
    $self Update $driverPtr
  }


  method Update driverPtr {
    set ctlr [::VMUSBDriverSupport::convertVmUSB $driverPtr]
  }

  method SetInspect value {
    $driver AccessBus 0x10000
    if {[catch {$driver WriteInspect $value} msg]} {
      $driver ReleaseBus
      return "ERROR Failure while writing inspect : $msg"
    }

    $driver ReleaseBus
    return OK
  }

  method SetBypass value {
    $driver AccessBus 0x10000
    if {[catch {$driver WriteBypass $value} msg]} {
      $driver ReleaseBus
      return "ERROR Failure while writing bypass : $msg"
    }

    $driver ReleaseBus
    return OK
  }

  method SetDelayWidth {param value} {
    set channel [$self ExtractChannelNumber $param]
    if {[catch {$self DecodeDelayAndWidth $value} values]} {
      return $values
    }

    set delay [lindex $values 0]
    set width [lindex $values 1]

    $driver AccessBus 0x10000
    if {[catch {$driver WriteDelayWidth $channel $delay $width} msg]} {
      $driver ReleaseBus
      return "ERROR Failed while writing delay width to the module. Msg=\"$msg\""
    }

    $driver ReleaseBus
    return OK
  }

  method DecodeDelayAndWidth value {
    # extract the delay and width 
    set pattern {^delay(\d+)width(\d+)$}
    set matches [regexp -inline $pattern $value]

    if {[llength $matches] == 0} {
      return -code error "ERROR Unable to parse delay and width values from value=\"$value\""
    }

    return [lreplace $matches 0 0]
  }

  method ExtractChannelNumber param {
    set pattern {^delaywidth(\d+)$}
    set channel [lindex [regexp -inline $pattern $param] 1]
    # make sure that the channel is not left padded with zeroes, because that
    # might end up causing the number to be treated as an octal number
    return [string trimleft $channel 0]
  }

  method IsDelayWidth {param} {
    return [expr {[$self ExtractChannelNumber $param] ne {}}]
  }

  method Set {ctlr param value} {
    # convert the ctlr to something usable
    set ctlr [::VMUSBDriverSupport::convertVmUSB $ctlr]

    $driver SetController $ctlr
  
    switch $param {
      inspect {
        return [$self SetInspect $value]
      }
      bypass  {
        return [$self SetBypass $value]
      }
      default {
        if {[$self IsDelayWidth $param]} {
          return [$self SetDelayWidth $param $value]
        } else {
          return "ERROR Parameter value \"$param\" is not supported for Set operation."
        }
      }
    } ;# end of switch

  } ;# end of Set


  method GetFirmware {} {
    $driver AccessBus 0x10000
    if {[catch {$driver ReadFirmware} msg]} {
      $driver ReleaseBus
      return "ERROR Failure while reading firmware signature : $msg"
    }
    $driver ReleaseBus
    return $msg
  }

  method GetInspect {} {
    $driver AccessBus 0x10000
    if {[catch {$driver ReadInspect} msg]} {
      $driver ReleaseBus
      return "ERROR Failure while reading inspect register : $msg"
    }
    $driver ReleaseBus
    return $msg
  }

  method GetBypass {} {
   
    $driver AccessBus 0x10000
    if {[catch {$driver ReadBypass} msg]} {
      $driver ReleaseBus
      return "ERROR Failure while reading bypass register : $msg"
    }
   
    $driver ReleaseBus
    return $msg
  }

  method GetDelayWidth param {
    set channel [$self ExtractChannelNumber $param]

    $driver AccessBus 0x10000
    if {[catch {$driver ReadDelayWidth $channel} msg]} {
      $driver ReleaseBus
      return "ERROR Failed while reading channel $channel delay/width from the module. Msg=\"$msg\""
    }

    $driver ReleaseBus
    return $msg
  }

  method Get {ctlr param} {

    set ctlr [::VMUSBDriverSupport::convertVmUSB $ctlr]
    $driver SetController $ctlr

    switch $param {
      fwsignature { return [$self GetFirmware]}
      inspect     { return [$self GetInspect]}
      bypass      { return [$self GetBypass]}
      default {
        if {[$self IsDelayWidth $param]} {
          return [$self GetDelayWidth $param]
        } else {
          return "ERROR Parameter value \"$param\" is not supported for Get operation."
        }
      }
    } ; # end of switch
  } ;# end of Get


  method addMonitorList aList {}

  method processMonitorList data {
    return 0
  }

}
