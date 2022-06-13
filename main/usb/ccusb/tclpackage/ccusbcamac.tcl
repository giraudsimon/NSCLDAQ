#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#       NSCL DAQ Development Group 
#       NSCL
#       Michigan State University
#       East Lansing, MI 48824-1321
# @author Jeromy Tompkins


package provide ccusbcamac 1.0

package require cccusb
package require cccusbreadoutlist
package require CCUSBDriverSupport 


## The ccusbcamac namespace
# 
# This package provides a purely tcl package for executing remote commands to 
# a CCUSB being run by the CCUSBReadout program. It is compatible with the 
# wienercamac package in that it presents the user with the same interface.
# There are a few differences that occur due to the nature of this being
# a client to the CCUSBReadout slow-controls server. The main one being that
# before any call to a cdreg is allowed, a call to cdconn needs to be 
# made with the same branch and crate indices. The reason for this is because
# the cdreg creates a new cccusb::CCCUSBRemote object that requires knowledge
# of the hostname, port, and module name. The purpose of cdconn is to register
# these values to a dictionary whose keys are uniquely formed by the b and c.
# If these have not been created, then there is not way for cdreg to acquire
# the information it needs to make a connection. 
#
# Because this is only the client side of the package, it is not enough to just
# require this package and start using it. For it to be effective, the user must
# add something of the following to their ctlconfig.tcl
#
# @verbatim
# Module create ccusb ctlr
# @endverbatim
#
# Adding such a line to the ctlconfig.tcl file and then running the CCUSBReadout
# program on localhost with the slow-controls server listening on port 27000, 
# you would establish a registry for a device in slot 12 as:
# 
# \code
# package require ccusbcamac
# 
# # Provide the connection info
# ccusbcamac::cdconn 0 1 localhost 27000 ctlr
#
# # Connect 
# set reg [ccusbcamac::cdreg 0 1 12]
# \endcode
# 
#
namespace eval ccusbcamac {
  
  variable connectionCount 0 ;#< increments for every call to cdreg to ensure
  variable connectionInfo "" ;#< dict of connection info mapped to id

  variable lastReg ""        ;#< maintains last reg produced by cdreg (useful for tests)


  
  
  ## @brief Register connection information for b and c
  #
  # The host, port, namd name values are stored in a dict
  # whose key is a unique number produced by b and c (@seealso _computeIndex)
  # It is required that this proc be called prior to calling
  # cdreg because without doing so, cdreg will not have all 
  # of the necessary information to connect to the slow-controls server. 
  # Connection information is stored as a list of the form {host port name}.
  # 
  #
  # @param b     branch index
  # @param c     crate index
  # @param host  name of host running CCCUSBReadout slow-controls server
  # @param port  port on which slow-controls server listens for new connections
  # @param name  name of module loaded into slow-controls server 
  #
  # @returns "" 
  #
  # Exceptions:
  # - if b and/or c are invalid values
  proc cdconn {b c host port name} {
    variable connectionInfo
    ::ccusbcamac::_checkValidBAndC $b $c
  
    # compute the absolute crate index
    set id [::ccusbcamac::_computeIndex $b $c]
    # register the list of information
    dict set connectionInfo $id [list $host $port $name]
  
  }
  
  ## @brief Create a registry for use in calling commands later
  #
  # @attention {ccusbcamac::cdconn must be called with desired b and c before this
  #              is ever called}
  # 
  # This actually creates a cccusb::CCCUSBRemote object using the info held
  # in the connectionInfo dictionary associated with the b and c. This could
  # promptly exit. The user can avoid this situation by calling ccusbcamac::isOnline b c
  # to safely determine whether the server is up and accepting connections. 
  # 
  # The registry produced by this will contain the name of the cccusb::CCCUSBRemote
  # object and the slot number.
  #
  # @param b  branch index
  # @param c  crate index
  # @param n  slot number 
  #
  # @return list
  # @retval {ccusbremote-name n}
  #
  # Exceptions:
  # - Slow controls server not running = FATAL ERROR!
  # - Error (code=1) when cdconn has not be called previously
  #
  proc cdreg {b c n} {
    variable connectionCount
    variable connectionInfo
    variable lastReg 
    
    set id [::ccusbcamac::_computeIndex $b $c]
  
    if {![dict exists $::ccusbcamac::connectionInfo $id]} {
      return -code error "ccusbcamac::cdreg no connection info found. User must call \"cdconn $b $c host port name\" before calling cdreg $b $c" 
    }
  
    # at this point is is gauranteed that cdconn was made
    set connInfo [dict get $connectionInfo $id]
    
    set host [lindex $connInfo 0]
    set port [lindex $connInfo 1]
    set name [lindex $connInfo 2]
    
    # throw an error if we cannot connect to the server 
    if {! [::ccusbcamac::isOnline $b $c]} {
      return -code error "ccusbcamac::cdreg failed to connect to server"
    }
    
    # connect! 
    set dev [cccusb::CCCUSBRemote ccremote$connectionCount $name $host $port]
    incr connectionCount  ;# make sure that the next 
    set lastReg [list $dev $n]  
    return $lastReg 
  } 
  
  
  ## @brief perform a 24-bit camac operation
  #
  # This parses the value of f in order to determine whether
  # the operation is a read, control,  or write. If the
  # the operation is a write, the d parameter must be specified
  # to succeed.  
  # 
  # @param reg  a registry produced by cdreg
  # @param f    the function code
  # @param a    the subaddress
  # @param d    value to write
  #
  # @returns list
  # @retval  {read_data q x} - for a read
  # @retval  {written_data q x} - for a write
  # @retval  {0 q x} - for a control
  #
  # Exception returns:
  # - error if operation is a write and d is not provided
  # 
  proc cfsa {reg f a {d ""}} {
  
    # given the value of the f argument, pass the
    # responsibility to one of the helper functions
  
    if {[::ccusbcamac::_isRead $f]} {
      return [ccusbcamac::_doRead24 $reg $f $a ]
  
    } elseif {[::ccusbcamac::_isWrite $f]} {
  
      if {$d ne ""} {
        return [ccusbcamac::_doWrite24 $reg $f $a $d]
      } else {
        return -code error "ccusbcamac::cfsa not provided data to write"
      }
  
    } else {
  
      return [ccusbcamac::_doControl $reg $f $a]
    } 
  }
  
  ## @brief perform a 16-bit camac operation
  #
  # This is identical to the cfsa proc except that it 
  # handles 16-bit operations on the data way. For 
  # control operations, there is actually no difference.
  # See cfsa for a detailed explanation of the behavior.
  #
  #
  proc cssa {reg f a {d ""}} {
  
    if {[::ccusbcamac::_isRead $f]} {
      return [ccusbcamac::_doRead16 $reg $f $a ]
  
    } elseif {[::ccusbcamac::_isWrite $f]} {
      if {$d ne ""} {
        return [ccusbcamac::_doWrite16 $reg $f $a $d]
      } else {
        return -code error "ccusbcamac::cssa not provided data to write"
      }
    } else {
  
      return [ccusbcamac::_doControl $reg $f $a]
    } 
  }
  
  ## @brief Immediately perform a qstop
  # 
  # The qstop operation is when the controller performs the function
  # specified by f and a. It then continues repeating this f and a
  # until a Q response does not return 1. This implementation also
  # allows the controller to limit the number of repeats to a 
  # maximum number and halt execution even if the last Q response
  # was 1.
  #
  # @attention The current implementation only performs 16-bit data
  #             transfers.
  #
  # @param reg  a device registry produced by cdreg
  # @param f    the function code
  # @param a    the sub address to repeat
  # @param maxn the maximum number of repeat allowed.
  #
  # @returns list 16-bit words read from the device
  proc qstop {reg f a {maxn ""}} {
  
    set ctlr [lindex $reg 0]
    set n    [lindex $reg 1]
  
    if {$maxn eq ""} {
      set maxn [expr 32*24] ;# assumes 32 operations per device in a full crate
    }
  
    cccusbreadoutlist::CCCUSBReadoutList aList
    aList addQStop $n $a $f $maxn
    set data [$ctlr executeList aList [expr 2*$maxn]]
  
    # data is type std::vector<uint16_t> and needs to be converted
    # to something a bit more usable. It is converted to a list.
    return [::CCUSBDriverSupport::shortsListToTclList data 1]
  }
  
  
  ## @brief Perform a Q-scan operation
  #
  # The Q-Scan is when the controller initiates a read with NAF and then repeats 
  # after incrementing the A if the Q and X responses were 1. If the Q response
  # returns 0 and X is 1, the A is reset to 0 and the N incremented. If the Q=1,
  # X=1, and the next A is going to be greater than 15, the A is reset to 0 and N
  # is incremented. This continues until an X response of 0 is received or a 
  # maximum number of transfers have been completed.
  #
  # @param reg  a device registry produced by cdreg
  # @param f    the function code
  # @param a    the subaddress
  # @param maxn the maximum number of operations to complete b/4 stopping
  #
  # @returns list of 16-bit words read from the device
  #
  proc qscan {reg f a {maxn ""}} {
    set ctlr [lindex $reg 0]
    set n    [lindex $reg 1]
    
    if {$maxn eq ""} {
      set maxn [expr 32*24] ;# assumes 32 operations per device in a full crate
    }
    
    cccusbreadoutlist::CCCUSBReadoutList aList
    aList addQScan $n $a $f $maxn
    set data [$ctlr executeList aList [expr 2*$maxn]]
   
    # convert std::vector<uint16_t> to a standard tcl list
    return [::CCUSBDriverSupport::shortsListToTclList data 1]
  
  }
  
  ## @brief Repeatedly execute an NAF a specific number of times
  #
  # This is like the QStop except that it unconditionally executes
  # a specific number of times and is ignorant of the Q response.
  #
  # @param reg  a device registry
  # @param f    the function code
  # @param a    the subaddress
  # @param num  the number of repetitions
  #
  # @returns list of 16-bit words read from device
  proc cblock {reg f a num} {
    set ctlr [lindex $reg 0]
    set n    [lindex $reg 1]
    
    cccusbreadoutlist::CCCUSBReadoutList aList
    aList addRepeat $n $a $f $num 
    set data [$ctlr executeList aList [expr 2*$num]]
  
    # convert std::vector<uint16_t> to tcl list of shorts
    return [::CCUSBDriverSupport::shortsListToTclList data 1]
  }
  
  
  ## @brief Check to see if the server is accepting connections
  #
  # More specifically, this determines whether the server will connect
  # one more connection. It does so by trying to connect. If a conneciton
  # is established, then the connection is closed and the proc 
  # returns 1. Otherwise, it returns 0. 
  #
  # @param b branch index. must be in range [0,7]
  # @param c crate index. must be in range [1,7]
  #
  # @return boolean
  # @retval 0 - offline (i.e. unable to connect to server)
  # @retval 1 - online 
  #
  # Exceptional returns:
  # - Error called when b and/or c are out of range
  # - Error called when ccusbcamac::cdconn has not be called already 
  proc isOnline {b c} {
    ::ccusbcamac::_checkValidBAndC $b $c
    # if here then b and c are good
  
    set id [::ccusbcamac::_computeIndex $b $c]
  
    # check whether cdconn has been called already. If it has
    # then a $id will be a key in the connectionInfo dict
    if {[dict exist $::ccusbcamac::connectionInfo $id]} {
  
      # get the connection info 
      set connInfo [dict get $::ccusbcamac::connectionInfo $id]
      set host [lindex $connInfo 0]
      set port [lindex $connInfo 1]
      
      # try to connect 
      if {[catch {socket $host $port} result]} {
        # we failed to connect
        return 0   
      } else {
        catch {close $result}
        return 1
      }
      
    } else {
      set msg "::ccusbcamac::isOnline has no connection information. "
      append msg "ccusbcamac::cdconn must be called prior to this with same b and c"
      return -code error $msg 
    }
  
  }
  
  
  ## @brief Return graded LAM register
  #
  # THIS IS NOT IMPLEMENTED b/c it doesn't make any sense for 
  # a single crate
  proc getGl {b} {
    puts "ccusbcamac::getGl is not implemented"
  }
  
  ## @brief Calls a C command on CAMAC dataway
  #
  # This does not assume that a cdreg has been called already
  # and demands that a cdconn has been called. It creates a
  # new connection and then executes the C using the 
  # connection object.
  #
  # @param b branch index. must be in range [0,7]
  # @param c crate index. must be in range [1,7]
  #
  # @returns ""
  #
  # Exceptional retunrns:
  # - Error when b and/or c are out of range
  proc C {b c} {
  
    # return errors if the b and c are not in range
    ::ccusbcamac::_checkValidBAndC $b $c
  
    # if here then b and c are good
    set id [::ccusbcamac::_computeIndex $b $c]
  
    # check to see if the $id is a key in the connectionInfo
    # to see if cdconn has been called
    if {[dict exist $::ccusbcamac::connectionInfo $id]} {
  
      # now it is possible to create a new connection to 
      set connInfo [dict get $::ccusbcamac::connectionInfo $id]
      set reg [ccusbcamac::cdreg $b $c 0]
      [lindex $reg 0] c 
    } else {
      set msg "::ccusbcamac::C connection info not provided. "
      append msg "cdconn needs to be called prior to calling this." 
      return -code error $msg 
    }
    
  }
  
  ## @brief Calls a Z command on CAMAC dataway
  #
  # This does not assume that a cdreg has been called already
  # and demands that a cdconn has been called. It creates a
  # new connection and then executes the Z using the 
  # connection object.
  #
  # @param b branch index. must be in range [0,7]
  # @param c crate index. must be in range [1,7]
  #
  # @returns ""
  #
  # Exceptional retunrns:
  # - Error when b and/or c are out of range
  proc Z {b c} {
  
    ::ccusbcamac::_checkValidBAndC $b $c
    # if here then b and c are good
  
    set id [::ccusbcamac::_computeIndex $b $c]
    if {[dict exist $::ccusbcamac::connectionInfo $id]} {
      set connInfo [dict get $::ccusbcamac::connectionInfo $id]
      set reg [ccusbcamac::cdreg $b $c 0]
      [lindex $reg 0] z
    } else {
      set msg "::ccusbcamac::Z connection info not provided. "
      append msg "cdconn needs to be called prior to calling this." 
      return -code error $msg
    }
  
  }
  
  ## @brief Check whether the crate is inhibited
  #
  # For firmware versions >= 0x08e000601, this actually calls the
  # CCCUSB::isInhibited function. For firmware versions less
  # than that, this returns an error.
  #
  # @param b    branch index
  # @param c    crate index
  #
  # @returns ""
  #
  proc isInhibited {b c} {
   
    ::ccusbcamac::_checkValidBAndC $b $c
    # if here then b and c are good
  
    set id [::ccusbcamac::_computeIndex $b $c]
  
    # check if cdconn has been called before
    if {[dict exist $::ccusbcamac::connectionInfo $id]} {
      set connInfo [dict get $::ccusbcamac::connectionInfo $id]
      set reg [ccusbcamac::cdreg $b $c 0]
  
      set ctlr [lindex $reg 0]

      set fwId [ $ctlr readFirmware ]
      if {$fwId >= 0x8e000601} {
        return [$ctlr isInhibited]
      } else {
        set msg    "::ccusbcamac::isInhibited unsupported for CC-USB firmware version "
        append msg [format "0x%x. Version must be >= 0x8e000601." $fwId]
        return -code error $msg
      }
    } else {
      set msg "::ccusbcamac::isInhibited connection info not provided. "
      append msg "cdconn needs to be called prior to calling this." 
      return -code error $msg
    }
  }
  
  
  ## @brief Inhibit/Unhibit the crate
  #
  # This either inhibits or uninhibits the crate depending
  # on the value of on
  # 
  # @param b  the branch address
  # @param c  the crate address
  # @param on inhibit (on=1), uninhibit (on=0) 
  #
  # @returns ""
  #
  # Exceptional returns:
  # - Error if b and/or c are out of range
  # - Error if cdconn has not be called before this
  proc Inhibit {b c on} {
  
    ::ccusbcamac::_checkValidBAndC $b $c
    # if here then b and c are good
  
    set id [::ccusbcamac::_computeIndex $b $c]
  
    # check if cdconn has been called before
    if {[dict exist $::ccusbcamac::connectionInfo $id]} {
      set connInfo [dict get $::ccusbcamac::connectionInfo $id]
      set reg [ccusbcamac::cdreg $b $c 0]
  
      set ctlr [lindex $reg 0]
      if {$on} {
         $ctlr inhibit
      } else {
         $ctlr uninhibit
      }
    } else {
      set msg "::ccusbcamac::inhibit connection info not provided. "
      append msg "cdconn needs to be called prior to calling this." 
      return -code error $msg
    }
  }
  
  
  ## @brief Read register stating which LAM lines are set
  # 
  # Queries the state of the LAM lines. The state of each LAM line is
  # encoded in the bits of the returned integer. LAM1 is in bit0, LAM2 in bit1,
  # etc. 
  #
  # @attention A call to this must be preceeded by a call to ccusbcamac::cdconn
  #            with the same b and c parameters.
  #
  # @param b  branch index
  # @param c  crate index
  #
  # @returns integer with state of LAM lines encoded in its bits
  #
  # Exceptional returns:
  # - Error if b and/or c are out of range
  # - Error if cdconn has not be called previously
  proc ReadLams {b c} {

    ::ccusbcamac::_checkValidBAndC $b $c
    # if here then b and c are good
  
    set id [::ccusbcamac::_computeIndex $b $c]
    if {[dict exist $::ccusbcamac::connectionInfo $id]} {
  
      set connInfo [dict get $::ccusbcamac::connectionInfo $id]
      set reg [ccusbcamac::cdreg $b $c 25]
  
      # CAMAC LAM pseudo register N(25)A(10)F(0) contains 
      # status of LAM lines. It is 24-bits wide.
#      return [ccusbcamac::_doRead24 $reg 0 10]
      set ctlr [lindex $reg 0]
      return [$ctlr readCAMACLams]
  
    } else {
      set msg "::ccusbcamac::ReadLams connection info not provided. "
      append msg "cdconn needs to be called prior to calling this." 
      return -code error $msg
    }
  
  }
  
  ## @brief Check whether f maps to a read function
  #
  # @param f the f value of an NAF command
  # 
  # @return bool
  # @retval 0 - is not a read command
  # @retval 1 - is a read command
  proc _isRead {f} {
    return [expr {$f>=0 && $f<8}]
  }

  ## @brief Check whether f maps to a write function
  #
  # @param f the f value of an NAF command
  # 
  # @return bool
  # @retval 0 - is not a write command
  # @retval 1 - is a write command
  proc _isWrite {f} {
    return [expr {$f>=16 && $f<24}]
  }


  ## @brief Compute the absolute crate index 
  #
  # First check to make sure that these are valid
  # values for b and c. If not, the user will be given
  # error messages explaining what happened.
  #
  # @param b branch index. must be in range [0,7]
  # @param c crate index. must be in range [1,7]
  # 
  # @returns int
  # @retval the absolute index of the crate
  proc _computeIndex {b c} {
  
    _checkValidBAndC $b $c

    return [expr {$b*7+$c}]
  }


  ## @brief Check that value is in range [0,7]
  # 
  # @param b branch index
  #
  # @returns boolean
  # @retval 0 - b is not in range [0,7]
  # @retval 1 - b is in range
  proc _isValidBranchIndex b {
    return [expr {$b>=0 && $b<8}]
  }

  ## @brief Check that value is in range [1,7]
  # 
  # @param c crate index
  # 
  # @returns boolean
  # @retval 0 - c is not in range [1,7]
  # @retval 1 - c is in range
  proc _isValidCrateIndex c {
    return [expr {$c>0 && $c<8}]
  }


  ##  @brief Produces errors if b and/or c are invalid
  # 
  # @param b branch index
  # @param c crate index
  # 
  # @retval  "" - b and c were in range
  # @retval  error message if b and/or c are out of range 
  # 
  proc _checkValidBAndC {b c} {
    # Check that the branch is in range
    set bIsGood [_isValidBranchIndex $b]
    set cIsGood [_isValidCrateIndex $c]

    if {!$bIsGood && !$cIsGood} {
      set msg "ccusbcamac::_checkValidBAndC : branch and crate indices out of range. "
      append msg {Branch should be in range [0,7] and crate in range [1,7]} 
      return -code error $msg
    }

    if {! $bIsGood} {
      return -code error {ccusbcamac::_checkValidBAndC : branch index out of range [0,7]}
    }

    # check that the branch is out of range
    if {! $cIsGood} {
      return -code error {ccusbcamac::_checkValidBAndC : crate index out of range [1,7]}
    }
  }

  ## @brief Utility method to perform a Read16 
  #
  # Extracts the ctlr and slot number from the registry object
  # and then calls a simpleRead16. It then decodes the datum,
  # Q, and X values from the returned integer. It then formats 
  # the returned result.
  #
  # @param reg  a device registry
  # @param f    function code
  # @param a    subaddress
  # 
  # @returns list
  # @retval {data q x}
  #
  proc _doRead16 {reg f a } {
    
    set ctlr [lindex $reg 0]
    set n    [lindex $reg 1]
  
    set result [$ctlr simpleRead16 $n $a $f]
  
    set data [expr {$result & 0xffff}]
    set q [expr {($result >> 24) & 0x1}]
    set x [expr {($result >> 25) & 0x1}]
  
    return [list $data $q $x]
  }
  
  ## @brief Utility method to perform a Read24 
  #
  # Extracts the ctlr and slot number from the registry object
  # and then calls a simpleRead24. It then decodes the datum,
  # Q, and X values from the returned integer. It then formats 
  # the returned result.
  #
  # @param reg  a device registry
  # @param f    function code
  # @param a    subaddress
  # 
  # @returns list
  # @retval {data q x}
  #
  proc _doRead24 {reg f a } {
    
    set ctlr [lindex $reg 0]
    set n    [lindex $reg 1]
  
    set result [$ctlr simpleRead24 $n $a $f]
  
    set data [expr {$result & 0xffffff}]
    set q [expr {($result >> 24) & 0x1}]
    set x [expr {($result >> 25) & 0x1}]
  
    return [list $data $q $x]
    
  }
  
  ## @brief Utility method to perform a 16-bit write
  #
  # Extracts the ctlr and slot number from the registry object
  # and then calls a simpleWrite16. It then decodes Q and X values 
  # from the returned integer and formats 
  # the returned result. It is assumed that the user has provided 
  # a valid value for all parameters.
  #
  # @param reg  a device registry
  # @param f    function code
  # @param a    subaddress
  # @param d    data to write 
  # 
  # @returns list
  # @retval {d q x}
  #
  proc _doWrite16 {reg f a d} {
    
    set ctlr [lindex $reg 0]
    set n    [lindex $reg 1]
  
    set result [$ctlr simpleWrite16 $n $a $f $d]
  
    # writes return just the q and x values
    set q [expr {($result & $::cccusb::CCCUSB_Q) == $::cccusb::CCCUSB_Q}]
    set x [expr {($result & $::cccusb::CCCUSB_X) == $::cccusb::CCCUSB_X}]
  
    return [list $d $q $x]
    
  }
  
  ## @brief Utility method to perform a 24-bit write
  #
  # Extracts the ctlr and slot number from the registry object
  # and then calls a simpleWrite24. It then decodes the Q and X values 
  # from the returned integer and formats 
  # the returned result. It is assumed that the user has provided 
  # a valid value for all parameters.
  #
  # @param reg  a device registry
  # @param f    function code
  # @param a    subaddress
  # @param d    data to write 
  # 
  # @returns list
  # @retval {d q x}
  #
  proc _doWrite24 {reg f a d} {
    
    set ctlr [lindex $reg 0]
    set n    [lindex $reg 1]
  
    set result [$ctlr simpleWrite24 $n $a $f $d]
  
    # writes returns just the q and x values
    set q [expr {($result & $::cccusb::CCCUSB_Q) == $::cccusb::CCCUSB_Q}]
    set x [expr {($result & $::cccusb::CCCUSB_X) == $::cccusb::CCCUSB_X}]
  
    return [list $d $q $x]
    
  }
  
  ## @brief Utility method to perform a control operation 
  #
  # Extracts the ctlr and slot number from the registry object
  # and then calls a simpleControl. It then decodes the Q and X values 
  # from the returned integer and formats 
  # the returned result. It is assumed that the user has provided 
  # a valid value for all parameters.
  #
  # @param reg  a device registry
  # @param f    function code
  # @param a    subaddress
  # 
  # @returns list
  # @retval {0 q x}
  #
  proc _doControl {reg f a} {
    
    set ctlr [lindex $reg 0]
    set n    [lindex $reg 1]
  
    set result [$ctlr simpleControl $n $a $f]
  
    set q [expr {($result & $::cccusb::CCCUSB_Q) == $::cccusb::CCCUSB_Q}]
    set x [expr {($result & $::cccusb::CCCUSB_X) == $::cccusb::CCCUSB_X}]
  
    return [list 0 $q $x]
    
  }
  

}
# -- END OF NAMESPACE --
