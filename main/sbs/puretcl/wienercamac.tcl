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


#
#
#   Package to support subset ESONE CAMAC access from TCL.
#   using the wiener vc32/cc32 cards...
#
#   Lives in the namespace ::camac::
#   Exported procs:
#       cdreg  b c n ?vme?            - Produce a handle to a camac module.
#       cfsa   reg f a ?d?       - Perform a 24 bit camac operation.
#       cssa   reg f a ?d?       - perform a 16 bit camac operation
#       qstop  reg f a ?maxn?    - Perform a qstop read.
#       qscan  reg f a ?maxn?    - Perform a qscan read
#       cblock reg f a num       - Perform a counted block transfer read.
# 
#  The following functions have no corresponding operation in the standard:
#
#       isOnline b c ?vme?             - Tests for crate online.
#       getGl    b  ?vme?             - Reads the branch graded lam register.
#
#  The following functions assume that the crate controller is a wiener cc32
#
#       C        b c ?vme?            - C cycle a crate
#       Z        b c ?vme?            - Z cycle a crate.
#       isInhibited b c   ?vme?       - Check inhibit status of crate.
#       Inhibit  b c  bool ?vme?      - Set crate inhibit.
#       ReadLams b c   ?vme?          - Read controller lam mask.
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


package provide wienercamac 1.1
package require Vme
package require camacutils

namespace eval wienercamac {


    #  caamac is an array with indHices of the
    #  form vmecrate,camaccrate
    #  containing vme mapping names of the form vc32_vme_camac
    #  where vme is a vme crate and camac is a camac crate number.
    #  these are created on demand via calls to BranchBase from
    #  cdreg.
    #


    variable camac
    set camac(dummy) ""

    proc BranchBase {crate {vme 0}} {
        variable camac
        
    
        # If the branch has not yet been mapped, do so on demand, and
        # save in the camac array:
        
        if {![info exists camac($vme,$crate)]} {
            append name vc32_ $vme _ $crate
            set camac($vme,$crate) \
            [vme create $name -device standard     \
                 -crate $vme                       \
                 [expr 0x800000 + $crate * 0x8000] \
                     0x80000]
            
        }
        return $camac($vme,$crate)
    }
    
    

    #  given a function code and a subaddress returns
    #  the address offset into a crate. Note that
    #  The 16's bit of f is not used.
    #
    proc Offset {f a} {
		set f [expr $f & 0xf];		# Remove 16's bit.
		return [expr ($a << 6) | ($f << 2)]
    }

#
# --ddc for cc32 naf for status is 0,0,0 (check status, q, x from cc32)
# q bit 3, x bit 2, inhibit bit 1, Lam bit 0
#
    proc CSR {b crate {vme 0}} {
        set module [cdreg $b 0 0 $vme]
    
    	set space [lindex $module 0]
	set base  [lindex $module 1]
    
    	set address [expr $base + [Offset 0 0]]


	set csrvalue [$space get -w $address]
	
    	return $csrvalue
    }
    proc Q {b crate {vme 0}} {
		set csr [CSR $b $crate $vme]    
		return [expr ($csr & 0x8) != 0]
    }
    proc X {b crate {vme 0}} {
		set csr [CSR $b $crate $vme]
		return [expr ($csr & 0x4) != 0]
    }
    #  The functions below make use of the fact that the first
    #  element of reg is of the form vc32_vme_camac
    #  (and that crate is always 0).

    proc ExtractB {reg} {

	   	set vmeid [lindex $reg 0]
		set list  [split $vmeid _]
		return    [lindex $list 2]
    }
    proc ExtractC {reg} {
		return 0
    }
    proc ExtractN {reg} {
		set offset [lindex reg 1]
		return [expr ($offset >> 10) & 0x1f]

    }
    proc ExtractV {reg} {
		set vmeid [lindex $reg 0]
		set list  [split $vmeid _]
		return    [lindex $list 1]
		   
    }
    proc IncN {reg} {
		set b [ExtractB $reg]
		set c [ExtractC $reg]
		set n [ExtractN $reg]
		incr n
		set v [ExtractV $reg]

		return [cdreg $b $c $n $v]
    }

	#
	#  cdreg b c n
	#   Produces a 'handle' to a module in camac space at b c n
	#   this handle should be passed into other procs which have a 'reg'
	#   parameter. Returns the handle.  Flags an error if any of b c n are
	#   invalid.
	#
	#   This proc returns a 2 element list.  The first element of the list
	#   is the name of the VME address space that is mapped to the crate.
	#   The second, the offset within that crate to the slot.
	proc cdreg {b c n {v 0}}  {
	
	    camacutils::isValid $b $c $n;			# Error's if invalid camac op.
	    set base    [BranchBase $b $v];	# Base id the crate controller.
	    set offset  [expr $n << 10];	# Offset to the slot.
	    

	    return [list $base $offset];	# Return {controler_id slot_offset}
	
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
        proc cfsa {reg f a {data 0} {size l}} {
	#
	#     Return errors if the fcode or subaddress are bad.
	#    
	    camacutils::isValidf $f
	    camacutils::isValida $a
	
	    set vmeid [lindex $reg 0];		# VME address id.
	    set module [lindex $reg 1];		# Base address of module within map.
	    set fna    [Offset $f $a];		# F.a offset into module.
	
	    if {$f & 0x10} {			# F16 bit set...
		set resultdata $data;		# It's a write access..
		$vmeid set -$size [expr $module +$fna] $data
	    } else {				# F61 bit not set...
		set resultdata [$vmeid get -$size [expr $module + $fna]];# It's a resd
	    }
	
	#    Now pick up the q and x and build the reply list:
	#--ddc ... use modified q/x procedures (supply additional crate param)
	#
	    set br    [ExtractB $reg]
	    set crate [ExtractC $reg]
	    lappend resultdata [Q $br $crate]
	    lappend resultdata [X $br $crate]
	
	    return $resultdata
	}
	#
	#  cssa reg f a ?data?
	#
	#    Same as cfsa but all data transfers are done as 16 bit transfers.
	#    Well actually, CAMAC doesn't distinguish between 16 and 24 bit transfers
	#    electrically, so we just do the deed with the bottom 16 bits of dat.
	#
	
	proc cssa {reg f a {data 0}} {
	#
	#     Return errors if the fcode or subaddress are bad.
	#    
	    set resultdata [cfsa $reg $f $a [expr $data & 0xffff] w]
	    set d          [lindex $resultdata 0];# This is the returned data.
	    set d          [expr $d & 0xffff]; 	  # Aren't we lazy!!
	
	    set result     [list $d [lindex $resultdata 1] [lindex $resultdata 2]]
	
	    return $result
	}
	#
	#  qstop reg f a ?maxn?
	#     Performs a qstop read on the single f a on the module designated by reg.
	#     The results of the read are placed in a list which is returned.
	#     maxn if supplied limits the number of reads which can be performed.
	#     f must be a read function. reads are 24 bit reads.
	#
	
	proc qstop {reg f a {maxn -1}} {
	
	    if {![camacutils::isRead $f]} {
			error "Qstop functions must be read operations"
	    }
	    set result ""
	
	#      Default of maxn = -1 will give lots of reps on this register:
	
	    while {$maxn != 0} {    
			set op [cfsa $reg $f $a]
			set q  [lindex $op 1]
			if {!$q} {
			    return $result
			}
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
	    if {![camacutils::isRead $f]} {
			error "Qscan functions must be read operations."
	    }
	    set result ""
	
	    while {$maxn != 0} {
			set op [cfsa $reg $f $a]
			set data [lindex $op 0]
			set q [lindex $op 1]
			set x [lindex $op 2]
			if {!$x} {
			    return $result
			}
			if {!$q} {
			    set a 0
			    set reg [IncN $reg]
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
	    if {![camacutils::isRead $f]} {
			error "Counted block operations must be reads"
	    }
	    set result ""
	    for {set i 0} {$i < $num} {incr i} {
			lappend result [lindex [cfsa $reg $f $a] 0]
	    }
	    return $result
	}
	#
	#   isOnline b c
	#     Informs you if the specified crate is online.
	#
	proc isOnline {b c {vme 0}} {
	# --ddc modified encode, and expr returned for vc32.
	
		set data  [CSR $b $c $vme]
		
		set moduletype [expr $data >> 12]
		if {$moduletype == 8} {   # Supposed to be module id.
			return 1;                  # 'branch' has 1 crate online 
		} else {
		    return 0;                  # If not id, 'branch' has no crates online.
		}
	
	}
	#
	#   getGl b 
	#     Get the graded lam register for the specified branch.
	#     this is a 24 bit item.m
	#     Since the wiener module has not graded lam register, we
	#     return 0.
	#
	# --ddc This no longer makes sense.  What should be done to fix it?
	proc getGl {b {vme 0}} {
		return 0    
	}
	#
	# C  b c   
	#     Performs a C cycle of the selected crate.
	#
	proc C {b c {vme 0}} {
	#--ddc modified NAF for cc32
	
	    set reg [cdreg $b $c 0 $vme]
	    cssa $reg 16 0 0
	}
	#
	#  Z b c
	#     performs a Z cycle of the selected crate
	#
	proc Z {b c {vme 0} } {
	#--ddc modified NAF for cc32
	    set reg [cdreg $b $c 0 $vme]
	    cssa $reg 16 1 0
	}
	#
	#  isInhibited b c
	#     Returns a bool indicating if the crate is inhibited.
	#
	proc isInhibited {b c {vme 0}} {
	#--ddc modified NAF for cc32
	#    set reg [cdreg $b $c 0 ]
	#    return [lindex [cssa $reg 0 0] 1]
	    set csr [CSR $b $c $vme]    
	    return [expr ($csr & 0x2) != 0]
	}
	#
	#  Inhibit b c bool
	#     If bool is true the selected crate is inhibited, otherwise it
	#     is uninhibited.
	#
	proc Inhibit {b c bool {vme 0}} {
	#--ddc modified NAF for cc32
	    set reg [cdreg $b $c 27 $vme]
	
	    if {$bool} {
                cssa $reg 16 0
	    } else {
                cssa $reg 16 1
	    }
	}
	#
	#   ReadLams b c
	#    Reads the graded lams on the specified crate:
	# --ddc modified for CC32, reads "lam-flipflop"
	#       this has not worked _yet_(8may02) for wiener vc32/cc32.
	proc ReadLams {b c {vme 0}} {
	    set reg [cdreg $b $c 28 $vme]
	    return [lindex [cfsa $reg 0 0] 0];     # LAM Flipflop is a0.
	}
}