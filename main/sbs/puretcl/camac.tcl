#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321


# (C) Copyright Michigan State University 1938, All rights reserved 
#
#
#   Package to support subset ESONE CAMAC access from TCL.
#   using the CES CBD8210 parallel branch driver.
#
#   Lives in the namespace ::camac::
#   Exported procs:
#       cdreg  b c n [vmecrate]  - Produce a handle to a camac module.
#       cfsa   reg f a ?d?       - Perform a 24 bit camac operation.
#       cssa   reg f a ?d?       - perform a 16 bit camac operation
#       qstop  reg f a ?maxn?    - Perform a qstop read.
#       qscan  reg f a ?maxn?    - Perform a qscan read
#       cblock reg f a num       - Perform a counted block transfer read.
# 
#  The following functions have no corresponding operation in the standard:
#
#       isOnline b c [vmecrate]  - Tests for crate online.
#       getGl    b [vmecrate]    - Reads the branch graded lam register.
#
#  The following functions assume that the crate controller is a BiRa 1302
#
#       C        b c [vmecrate]       - C cycle a crate
#       Z        b c [vmecrate]                - Z cycle a crate.
#       isInhibited b c [vmecrate]    - Check inhibit status of crate.
#       Inhibit  b c  bool [vmecrate] - Set crate inhibit.
#       ReadLams b c  [vmecrate]      - Read controller lam mask.
# NOTE:
#   the optional vmecrate parameter in the functions below defaults to 0 
#   for compatibility with existing software.
#

#  Set up the auto_path so that it includes the canonicalized dir
#  above this package's location if necessary.
#  this should ensure that we'll find the Vme
#   package.

set here    [file dirname [info script]]
set libDir  [file normalize [file join $here ..]]

if {[lsearch $auto_path $libDir] == -1} {
    set auto_path [concat $libDir $auto_path]
}

package provide camac 1.1;		#  Multicrate version
package require Vme

namespace eval camac {

#    camac will describe a mapping of the entire branch highway space.

#    variable camac [vme create ces8210 -device /dev/vme24d16 0x800000 0x400000]
#
#    Camac is an array that will be idexed by branch/vme crate.
#    Each element of the array will select the mapping for a single
#    branch highway module.  Elements will be created on demand.
#
    variable camac
    set camac(nonsense) "";		# Ensure the array is created.

    namespace export cdreg cfsa cssa qstop qscan cblock
    namespace export isOnline getGl
    namespace export C Z isInhibited Inhibit ReadLams

    # Suports the map on demand of CAMAC branches:
    #   vme    - Vme crate.
    #   branch - Branch controller module in crate.
    # Returns the map base vme identifier of the 
    # branch.
    #
    proc BranchBase {vme branch} {
	variable camac

	#  If the branch has not been mapped yet, map on demand:


	if {![info exists camac($vme,$branch)]} {
	    append name ces8210_ $vme _ $branch
	    set camac($vme,$branch) \
		[vme create $name -device standard -crate $vme \
		     [expr 0x800000 + ($branch << 19)] 0x80000]

	}
	return $camac($vme,$branch) 
    }

    proc Encode {c n {a 0} {f 0}} {
        set reg [expr ($c << 16) | ($n << 11)]
        set reg [expr $reg | ($a << 7) | ($f << 2)]
        return $reg
    }
   
    proc Offset {a f} {
    	return [expr ($a << 7) | ($f << 2)]
    }

    proc CSR {b {vme 0}} {
	set reg [camac::cdreg $b 0 29 $vme]
	set branch [lindex $reg 0]
	set base   [lindex $reg 1]

	return [$branch get -w [expr $base +2]]

    }
    proc Q {b {vme 0}} {
	set csr [CSR $b $vme]    
	return [expr ($csr & 0x8000) != 0]
    }
    proc X {b {vme 0}} {
	set csr [CSR $b $vme]
	return [expr ($csr & 0x4000) != 0]
    }
    # Extract the Branch from a VME identifier.
    # VME identifiers are of the form ces8210_vme_branch
    #
    
    proc ExtractB {reg} {
		set decoded [split [lindex $reg 0] "_"]
	
		return  [lindex $decoded 2]
    }
    #  Extract the VME crate from a VME identifier
    #  VME Idents are of the form ces8210_vmme_branch
    proc ExtractC {reg} {
		set decoded [split [lindex $reg 0]  "_"]
		return [lindex $decoded 2]
    }

    proc ExtractN {reg} {
		set base [lindex $reg 1]
	
		return [expr ($base >> 11) & 0x1f]
    }
    proc IncN {reg} {
		set b [ExtractB $reg]
		set c [ExtractC $reg]
		set n [ExtractN $reg]
		incr n
		return [cdreg $b $c $n]   ;# Error if n now out of range!
    }
    proc ExtractVME {reg} {
		set decoded [lindex $reg 0]
		set splitdecoded [split $decoded "_"]
		return [lindex $splitdecoded  1]
    }
    ##
    # getXQ
    #  Given a branch identifiction, return
    #  returns a list that consists of the most recent q,x
    #  response.
    #
    # @param branch branch id to check.
    # @return list - 2 element list consisting of Q,X in that order.
    #
    proc getXQ {branch} {
        set B [camac::ExtractB      $branch]
        set V [camac::ExtractVME    $branch]
        
        lappend data [camac::Q $B $V]
        lappend data [camac::X $B $V]
        return $data
    }

    #
    #  cdreg b c n
    #   Produces a 'handle' to a module in camac space at b c n
    #   this handle should be passed into other procs which have a 'reg'
    #   parameter. Returns the handle.  Flags an error if any of b c n are
    #   invalid.
    #
    #    b - Branch 
    #    c - crate
    #    n - slot
    #  vme - Vme crate in which the branch controller is installed.
    #        Defaults to 0 for compatibility with existing software.
    #
    #  Returns:
    #    A handle that should be used to reference the slot.
    #    NOTE: this handle is actually a 2 element list:
    #          the first element is the handle to the VME
    #          space map for the branch.
    #          The second element is the offset into this map
    #          for the module's base.
    #
    proc cdreg {b c n {vme 0}}  {
	
	camacutils::isValid $b $c $n       ;# Error's if invalid camac op.
	set branch [camac::BranchBase $vme $b]
	set offset [camac::Encode $c $n]
	
	return [list $branch $offset]
	
	
    }
    #
    # cfsa reg f a ?d?
    #     Performs a single camac operation.  If the operation involves
    #     data transfer 24 bits are transferred.  If the operation is a 
    #     write, d contains the data to write and the low order 24 bits are 
    #     written..
    #     proc returns a list of the form:
    #       {data q x}
    #     data is:
    #        0 - If the operation was a control operation.
    #        d - If the operation was a write operation.
    #        ? - The data read from the module if the operation was a read.
    #
    #      reg - A module handle gotten from cdreg.
    #
    proc cfsa {reg f a {data 0}} {

        #
        #     Return errors if the fcode or subaddress are bad.
        #    
        camacutils::isValidf $f
        camacutils::isValida $a
        
        # Extract the VME space and module offset from the handle:
        
        set branch [lindex $reg 0]
        set module [lindex $reg 1]
        
        
        set address [expr $module + [camac::Offset $a $f]]
        
        
        if {[camacutils::isRead $f]} {		;# Read operation for 32 bits:
            set hi [$branch get -w $address]
            set lo [$branch get -w [expr $address + 2]]
            set data [expr (($hi & 0xff) << 16) | ($lo & 0xffff)]
        }
        if {[camacutils::isWrite $f]} {		;# Write operation for 32 bits:
            set hi [expr (($data >> 16) & 0xff)]
            set lo [expr $data & 0xffff]
            $branch set -w $address $hi
            $branch set -w [expr $address + 2] $lo
            set data $data
        }
        
        
        if {[camacutils::isCtl   $f]} {		# Control function.
            $branch  get -w [expr $address + 2]
            set data 0
        }
        
        #    Now pick up the q and x and build the reply list:
        
        set data [concat $data [getXQ $branch]]
        
	return $data
    }
    #
    #  cssa reg f a ?data?
    #
    #    Same as cfsa but all data transfers are done as 16 bit transfers.
    #    reg - is the handle for the module.
    #    f   - the function code.
    #    a   - the subadress
    #    data- optional data.
    #
    proc cssa {reg f a {data 0}} {
	#
	#     Return errors if the fcode or subaddress are bad.
	#    
	camacutils::isValidf $f
	camacutils::isValida $a
	
	# Extract vme address space and module base:
	
	set branch [lindex $reg 0]
	set base   [lindex $reg 1]
	set offset [camac::Offset $a $f]
	set address [expr $base + $offset +2 ]
       



	if {[camacutils::isRead $f]} {		;# Read operation for 32 bits:
	    set data [$branch get -w $address]
	}
	if {[camacutils::isWrite $f]} {		;# Write operation for 32 bits:
	    set lo [expr $data & 0xffff]
	    $branch set -w $address $lo
	    set data $data
	}
	if {[camacutils::isCtl   $f]} {		# Non transfer operation.
	    $branch get -w $address
	    set data 0
	}

	#    Now pick up the q and x and build the reply list:
	
	
    set data [concat $data [getXQ $branch]]
    
	return $data
    }
    #
    #  qstop reg f a ?maxn?
    #     Performs a qstop read on the single f a on the module designated by reg.
    #     The results of the read are placed in a list which is returned.
    #     maxn if supplied limits the number of reads which can be performed.
    #     f must be a read function. reads are 24 bit reads.
    #
    
    proc qstop {reg f a {maxn -1}} {
	
	if {![::camacutils::isRead $f]} {
	    error "Qstop functions must be read operations"
	}
	set result ""
	
	#      Default of maxn = -1 will give lots of reps on this register:
	
	while {$maxn != 0} {    
	    set op [camac::cfsa $reg $f $a]
	    set q  [lindex $op 1]
	    if {!$q} {return $result}
	    lappend result [lindex $op 0]
	    incr maxn -1
	}
	return $result
}
    #
    #  qscan reg f a ?maxn?
    #    Performs a qscan read operation.  Qscans read a module incrementing
    #    the subaddress after each operation.  If Q goes away or a >16
    #    the scan continues with the next slot.  If X goes away, the scan
    #    terminates.  The scan also terminates if the optional maxn
    #    count is exceeded.
    #
    proc qscan {reg f a {maxn -1}} {
	if {![::camacutils::isRead $f]} {
	    error "Qscan functions must be read operations."
	}
	set result ""
	
	while {$maxn != 0} {
	    set op [camac::cfsa $reg $f $a]
	    set data [lindex $op 0]
	    set q [lindex $op 1]
	    set x [lindex $op 2]
	    if {!$x} {return $result}
	    if {!$q} {
            set a 0
            set reg [::camac::IncN $reg]
            continue
	    }
	    lappend result $data
	    incr a
	    incr maxn -1
	}
	return $result
    }
    #
    #  cblock reg f a num
    #    Peforms an unconditional block read of num items from the 
    #    same camac bcnaf.  The data read are returned as a list.
    #
    proc cblock {reg f a num} {
	if {![::camacutils::isRead $f]} {
	    error "Counted block operations must be reads"
	}
	set result ""
	for {set i 0} {$i < $num} {incr i} {
	    lappend result [lindex [::camac::cfsa $reg $f $a] 0]
	}
	return $result
    }
    #
    #   isOnline b c
    #     Informs you if the specified crate is online.  Note that the 
    #     value is undefined if there is no branch controller for branch b.
    #
    proc isOnline {b c {v 0}} {
	set reg [::camac::cdreg $b 0 29 $v]
	set branch [lindex $reg 0]
	set base   [lindex $reg 1]
	set address [expr $base + [camac::Offset 0 9]]
	
	set onlines [$branch get -w [expr $address +2]]
	
	return [expr ($onlines & (1 << $c)) != 0]
    }
    #
    #   getGl b 
    #     Get the graded lam register for the specified branch.
    #     this is a 24 bit item.m
    #
    proc getGl {b {v 0}} {
	set reg [::camac::cdreg $b 0 29 $v]
	set branch [lindex $reg 0]
	set base   [lindex $reg 1]
	set address [expr $base + [camac::Offset 0 10]]
	
	set glhi [$branch get -w $address]
	set gllo [$branch get -w [expr $address + 2]]
	
	return [expr ($gllo &0xffff) | (($glhi && 0xff) << 16)]
	
    }
    #
    # C  b c   
    #     Performs a C cycle of the selected crate.
    #
    proc C {b c {v 0}} {
	set reg [::camac::cdreg $b $c 28 $v]
	::camac::cssa $reg 26 9
    }
    #
    #  Z b c
    #     performs a Z cycle of the selected crate
    #
    proc Z {b c {v 0}} {
	set reg [::camac::cdreg $b $c 28 $v]
	::camac::cssa $reg 26 8
    }
    #
    #  isInhibited b c
    #     Returns a bool indicating if the crate is inhibited.
    #
    proc isInhibited {b c {v 0}} {
	set reg [::camac::cdreg $b $c 30 $v]
	return [lindex [::camac::cssa $reg 27 9] 1]
    }
    #
    #  Inhibit b c bool
    #     If bool is true the selected crate is inhibited, otherwise it
    #     is uninhibited.
    #
    proc Inhibit {b c bool {v 0}} {
	set reg [::camac::cdreg $b $c 30 $v]
	if {$bool} {
	    ::camac::cssa $reg 26 9
	} else {
	    ::camac::cssa $reg 24 9
	}
    }
    #
    #   ReadLams b c
    #    Reads the graded lams on the specified crate:
    #
    proc ReadLams {b c {v 0}} {
	set reg [::camac::cdreg $b $c 30 $v]
	return [::camac::cfsa $reg 0 0]
    }
    
}
