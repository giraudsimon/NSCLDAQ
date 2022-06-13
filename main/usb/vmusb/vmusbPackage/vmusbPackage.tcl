#!/bin/sh
# -*- tcl -*-
# The next line is executed by /bin/sh, but not tcl \
exec tclsh "$0" ${1+"$@"}

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321



##
# @file vmusbPackage.tcl
# @brief VMUSB command line package that imitates SBS vme package.
# @author Ron Fox <fox@nscl.msu.edu>
#

package provide vmusbvme  1.0

#  Establish the  search path and load the VMUSB swig wrapper:

namespace eval vme {
    set here [file dirname [info script]]
    set libdir [file normalize [file join $here .. .. lib]]
    lappend ::auto_path $libdir
}

package require cvmusb
package require snit

##
# @class
#  vme::VMUSBManager
#    Handler for VMUSB devices.  It maintains exactly one VMUSB object for each
#    open device.  Devices are held open until this is deleted.  Note that
#    there is a vme::VMUSBManagerSingleton that should actually be used.
#
snit::type vme::VMUSBManager {
    
    # Array of CVMUSB's indexed by crate number.
    
    variable devices -array [list]
    
    
    ##
    # destructor
    #   close all vmusb devices that are open by destroying their underlying
    #   device and destroying the devices array.
    #
    destructor {
        foreach d [array names devices] {
            cvmusb::delete_CVMUSB $devices($d)
        }
        array unset devices *
    }

    
    ##
    # controller
    #   Return a specific controller object.
    #
    # @param crateNo   - the crate number.
    # @return controller object that can be used to perform VME operations.
    # @note If no controller object has been made for this crate one is created
    #       if possible.  See the errors however.
    # @throws If there is no crate by this number
    #
    method controller crateNo {
        if {[array names devices $crateNo] eq ""} {
            $self _openCrate $crateNo
        }
        return $devices($crateNo)
    }
    #--------------------------------------------------------------------------
    # Private methods
    #
    
    ##
    # _openCrate
    #    Opens a specific VME crates, saving the object in the devices array
    #
    # @param crateNo 0 - number of the crate to open.
    #
    method _openCrate crateNo {
        set enumerationVector [::cvmusb::CVMUSB_enumerate]
        if {$crateNo < [::cvmusb::usb_device_vector_size $enumerationVector]} {
            set devices($crateNo) [cvmusb::new_CVMUSB \
                [::cvmusb::usb_device_vector_get $enumerationVector $crateNo]]
        } else {
            error "There is no VME crate $crateNo"
        }
    }
}

snit::type vme::VMUSBManagerSingleton {
    component manager
    delegate method * to manager
    
    typevariable managerName ""
    
    constructor args {

        # Create the singleton the first time..
        
        if {$managerName eq ""} {
            set managerName [vme::VMUSBManager %AUTO%]
        }
        # Install the singleton as the component to which all things are
        # delegated.
        #
        install manager using set managerName
    }
}


#
# vme::Window
#    Represents and address window
#    An address window consists of
#    *  A base address
#    *  A size of the window in bytes.
#    *  An address modifier used to perform read/write operations.
#    *  A controller used to perform read/write operations.
#
# METHODS:
#    -   peek  - reads from the window.
#    -   poke  - writes to the window.
#
snit::type vme::Window {
    variable base
    variable size
    variable am
    variable controller
    
    ##
    # Constructor:
    #   @param baseAddr - base address of the map
    #   @param nBytes   - length of the map.
    #   @param amod     - address modifier to use
    #   @param crate    - Crate number.
    #
    constructor {baseAddr nBytes amod crate} {
        set base $baseAddr
        set size $nBytes
        set am   $amod
        
        set manager [vme::VMUSBManagerSingleton %AUTO%]
        set controller [$manager controller $crate]
    }
    
    ##
    # peekl
    #
    #  @param offset  - Offset from window start for the peek.
    #  @return value  - Longword value read from that offset.
    #
    method peekl offset {
        set address [$self _address $offset]
        return [$controller vmeRead32 $address $am]
    }
    
    ##
    # peekw
    #
    #  @param offset  - Offset from window start for the peek.
    #  @return value  - 16 bit value read from that offset.
    #
    method peekw offset {
        set address [$self _address $offset]
        return [$controller vmeRead16 $address $am]
    }
    
    
    ##
    # peekb
    #
    #  @param offset  - Offset from window start for the peek.
    #  @return value  - 8 bit value read from that offset.
    #
    method peekb offset {
        set address [$self _address $offset]
        return [$controller vmeRead8 $address $am]
    }

    
    ##
    # pokel
    #
    # @param offset - offset into the window.
    # @param value  - value to write.
    #
    method pokel {offset value} {
        set address [$self _address $offset]
        return [$controller vmeWrite32 $address $am $value]
    }
    
    
    ##
    # pokew
    # @param offset - offset into the window.
    # @param value  - value to write.
    #
    method pokew {offset value} {
        set address [$self _address $offset]
        return [$controller vmeWrite16 $address $am $value]
    }
    ##
    # pokeb
    # @param offset - offset into the window.
    # @param value  - value to write.
    #
    method pokeb {offset value} {
        set address [$self _address $offset]
        return [$controller vmeWrite8 $address $am $value]
    }
    
    ##
    # get
    #   Generate the correct peek operation and perform it
    #
    # @param type (-l  -w or -b)
    # @param offset  offset into window.
    #
    method get {Type offset} {
        if {$Type in [list -l -w -b]} {
            set method peek[string range $Type 1 end]
            return [$self $method $offset]
        } else {
            error "Invalid width specification $type"
        }
    }
    ##
    # put
    #   Generic poke like get above.
    # @param type
    # @param offset
    # @param value
    #
    method put {Type offset value} {
        if {$Type in [list -l -w -b]} {
            set method poke[string range $Type 1 end]
            return [$self $method $offset $value]
        } else {
            error "Invalid width specification $type"
        }
        
    }
    
    #-------------------------------------------------------------------------
    # Private methods
    #
    
    ##
    # _address
    #   Ensure an offset is within the window
    #
    # @param offset  - Address offset.
    # @return address of the operation.
    # @throw error if the offset is out of range.
    #
    method _address offset {
        if {$offset < $size} {
            return [expr {$base + $offset}]
        } else {
            error "$offset is out of range of the window."
        }
    }
    
    
    
}

#------------------------------------------------------------------------------
#  Public part of the package.  We provide a namespace ensemble vme
#  with sub commands:
#  *  enumerate  - returns a mapping of crate number to serial number and vica versa.
#  *  create     - Creates an addresss window in which commands can be done:
#  *  list       - Lists the adddress windows that are active.
#  *  delete     - Destroys an existing map.
#
#  VME address windows become command ensembles with the sub commands:
#  * put            - Puts a value to a cell in the window.
#  * get            - Gets a value from a cell in the window.
#






namespace eval vme {

    variable addressModifiers
    array set addressModifiers [::list extended 0x3d standard 0x39 shortio 0x29 geo 0x2f]
    
    variable windows [::list]
    
    ##
    # enumerate
    #    Provides a mapping between VME serial numbers and VME crate numbers
    #    Note that this provides the mapping of the moment.  See, however the
    #    CVMUSB class above for why it's important not to have VME hot (un)plugs
    #    while the application is still opening devices.
    #
    # @return list of pairs, the first element of each pair is the crate number
    #         the second, the serial number of that crate controller.
    #
    proc Enumerate {} {
        set enumerationVector [::cvmusb::CVMUSB_enumerate]
        set result [::list]
        set enumerationSize [::cvmusb::usb_device_vector_size $enumerationVector]
        for {set i 0} {$i < $enumerationSize} {incr i} {
            set device [::cvmusb::usb_device_vector_get $enumerationVector $i]
            set serno  [::cvmusb::string_to_char [::cvmusb::CVMUSB_serialNo $device]]
            lappend result [::list $i $serno]
        }
        return $result
    }
    ##
    # create
    #   Create a new Window object as directed by the parameters.
    #
    # @param args
    #
    # @return name of the ap.
    #
    proc Create args {
        
        set name [lindex $args 0]
        set args [lrange $args 1 end]
        
        # Need at least two args and even number:
        
        set nargs [llength $args]
        if {($nargs < 2) || (($nargs %2) != 0)} {
            error "create requires an even number of at least two parameters."
        }
        
        # Set default options:
        
        set options(-device) standard
        set options(-crate)  0
        
        #  Iterate through any options:
        
        for {set i 0} {$i < $nargs} {incr i 2} {
            set opname [lindex $args $i]
            set opval  [lindex $args [expr {$i+1}]]
            if {$opname in  {-device -crate}} {
                set options($opname) $opval
            } else {
                break
            }
        }
        # I is supposed to be the index of the base address now:
        
        set base [lindex $args $i]
        incr i
        set size [lindex $args $i]
        set am $vme::addressModifiers($options(-device))
        
        # Create the window
        
        vme::Window ::$name $base $size $am $options(-crate)
        lappend vme::windows [::list $name $base]
        return ::$name
        
    }
    ##
    # list
    #   List the address windows:
    #
    proc List {} {
        return $vme::windows
    }
    ##
    # Delete
    #   Kill off an existing address map:
    #
    # @param name - map name.
    #
    proc Delete name {
        set findIdx [lsearch -exact $name $::vme::windows]
        if {($findIdx == -1) || (($findIdx % 2) != 0)} {
            $name destroy;            # kill the command.
            set ::vme::windows [lreplace $::vme::windows $findIdx [incr findIdx]]
        } else {
            error "No such map $name"
        }
    }
    
    
  
    
}

proc vme args {
    set subcommand [string totitle [lindex $args 0]]
    set args [lrange $args 1 end]
    vme::$subcommand {*}$args
}