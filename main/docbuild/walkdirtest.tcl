set testing 1

package require tcltest
namespace import ::tcltest::*


source docbuilder.tcl

#
#  Replace the incorporateFragment
#  proc so that we can record what's called.
#
rename incorporateFragment old_incorporateFragment

set fragmentFiles [list]

proc incorporateFragment xmlFile {
    lappend ::fragmentFiles $xmlFile
}

test walk-1 {Walk an empty directory tree}     \
    -setup {
	set fragmentFiles [list]
	makeDirectory fragments
    }                                             \
    -cleanup {
	removeDirectory fragments
	set fragmentFiles [list]
    }                                             \
    -body {
	walkDirectory fragments
	set fragmentFiles
    }                                             \
    -result {}

test walk-1 {Walk a single level directory tree with a fragment} \
    -setup {
	set fragmentFiles [list]
	makeDirectory fragments
	makeFile "" [file join fragments test.xml]
	makeFile "" [file join fragments a.xml]
    }                                                            \
    -cleanup {
	removeFile [file join fragments test.xml]
	removeFile [file join fragments a.xml]
	removeDirectory fragments
	set fragmentFiles [list]
    }                                                            \
    -body {
	walkDirectory fragments
	set fragmentFiles
    }                                                            \
    -result {fragments/a.xml fragments/test.xml}

test walk-2 {Walk a dual level directory tree with fragments at each level } \
    -setup {
	set fragmentFiles [list]
	makeDirectory fragments
	makeFile "" [file join fragments test.xml]
	makeFile "" [file join fragments a.xml]
	set s1      [file join fragments s1]
	makeDirectory $s1
	makeFile "" [file join $s1 s1.xml]
	set s2      [file join fragments s2]
	makeDirectory $s2
	makeFile "" [file join $s2 s2.xml]
    }                                                                        \
    -cleanup {
	removeFile [file join $s2 s2.xml]
	removeFile [file join $s1 s1.xml]
	removeDirectory $s1
	removeDirectory $s2
	removeFile [file join fragments test.xml]
	removeFile [file join fragments a.xml]
	removeDirectory fragments
	set fragmentFiles [list]

    }                                                                        \
    -body {
	walkDirectory fragments
	set fragmentFiles
    }                                                                        \
    -result {fragments/a.xml fragments/test.xml fragments/s1/s1.xml fragments/s2/s2.xml}

rename incorporateFragment {}
rename old_incorporateFragment incorporateFragment
cleanupTests