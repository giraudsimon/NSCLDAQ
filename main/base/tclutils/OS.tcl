#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#

#
#  The following are Operating system specific functions.
#

# Recent work in this module has been to maket his software more
# closely match the suggested Tcl coding standard.

package provide OS 2.0

namespace eval OS {
    variable OSType $tcl_platform(os)   ;# Current operating system.
    variable OSList                     ;# List of supported os's.
}
# OS::AddOs os
#
#  Add an operating system to the list of supported operating systems.
#
# Adds an operating system specific package to the list of systems this
# software can support.
#
# parameters:
#    os    - Name of the system (usually should be gotten via $tcl_platform(os)
#            or [exec uname] on unix like systems.
#
proc OS::AddOs     {os}  {
    lappend OS::OSList $os
}
#  OS::OsPkg  - Return the namespace for the specified
#               operating system.
#
proc OS::OsPkg {} {
    if {[lsearch -exact  $OS::OSList $OS::OSType] == -1} {
        error "OS::OsPkg - no package to support system: $OS::OSType"
    }
    return $OS::OSType
}
#  OS::DiskSpace dir
#       Return the size of the disk device associated with
#       specified path.
# Parameters:
#     dir   - Directory to search.
#
proc OS::DiskSpace {dir} {	;# Delegates function to os specific pkg.
    return [eval [OS::OsPkg]::DiskSpace $dir]
}
# OS::DiskSpaceWLinks dir
#        Return the disk space usage by a specified subtree of the file system
#        including any space used by files pointed to by soft links or their
#        equivalent.  A softlink is defined as anything that returns no-error
#        from [file readlink]
# Parameters:
#     dir    - Top of the subtree to total space usage for.
#
proc OS::DiskSpaceWLinks {dir} {
    return [eval [OS::OsPkg]::DiskSpaceWLinks $dir]
}
# OS::FreeSpace dir
#      Return the amount of disk space free on the device on which
#      the path specified by the directory $dir lives.
# Parameters:
#    dir   - Directory to calculate.
#
proc OS::FreeSpace {dir} {
    return [eval [OS::OsPkg]::FreeSpace $dir] ;# Delegates to os specific pkg.
}
# OS::SizeDir dir
#       Returns the space used by files under the designated directory subtree.
#       exluding files pointed to by symlinks or their operating system equivalent.
#
proc OS::SizeDir {dir} {
    return [eval [OS::OsPkg]::SizeDir $dir]   ;# Delegate to os pkg.
}

proc OS::TapeList {} {
    return [eval [OsPkg]::TapeList]
}
namespace eval OS {
    namespace export DiskSpace FreeSpace SizeDir DiskSpaceWL<inks
}
#
#   This namespace provides operating system specific
#   implementation for the OS:: functions for generic unix
#   systems.  In most cases, unix like systems can delegate
#   to these functions.
#
#   See OS:: functions of the same name for comments.
#
namespace eval Unix {}

proc Unix::DiskSpace {dir} {
    set df [exec df -k $dir]
    return [lindex $df 8]
}

set ::Unix::gnudu /usr/local/bin/du
proc Unix::DiskSpaceWLinks {dir} {
    set result [exec $::Unix::gnudu -skcL $dir | tail -1]
    return [lindex $result 0]
}
proc Unix::FreeSpace {dir} {
    set df [exec df -k  $dir]
    return [lindex $df 10]
}
proc Unix::SizeDir {dir} {
    return [lindex [exec du -sk $dir] 0]
}
proc Unix::TapeList {} {
    error "There is no generic way to get a list of unix tape devs"
}

#
#  Implementation of the OS:: package for Linux.
#
namespace eval Linux {}

proc Linux::DiskSpace {dir} {
    return [Unix::DiskSpace $dir]
}
proc Linux::DiskSpaceWLinks {dir} {
    set saveddu $::Unix::gnudu
    set ::Unix::gnudu /usr/bin/du
    set total [::Unix::DiskSpaceWLinks $dir]
    set ::Unix::gnudu $saveddu

    return $total

}
proc Linux::FreeSpace {dir} {
    return [Unix::FreeSpace $dir]
}
proc Linux::SizeDir {dir} {
    return [Unix::SizeDir $dir]
}
proc Linux::TapeList {} {
    return [glob -nocomplain /dev/nst*]
}

OS::AddOs Linux
#
#  Implementation of the OS:: Package for osf1
#  (e.g Tru64 unix).
#  This is deprecated.. note that the implementation of
#   OFS1::TapeList is incomplete and it is not planned
#  to complete it.
#
namespace eval OSF1  {}
proc OSF1::DiskSpace {dir} {
    return [Unix::DiskSpace $dir]
}
proc OSF1::DiskSpaceWLinks {dir} {
    return [Unix::DiskSpaceWLinks $dir]
}
proc OSF1::FreeSpace {dir} {
    return [Unix::FreeSpace $dir]
}
proc SizeDir {dir} {
    return [Unix::SizeDir $dir]
}

proc OSF1::TapeList {} {		;# Tru64 tapes depend on version.
    global tcl_platform
    set vers $tcl_platform(osVersion)
    switch $vers {
    }
}

OS::AddOs  OSF1
