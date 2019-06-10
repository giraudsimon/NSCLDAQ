#
#  Tests for processMetaDirectory
#

set testing 1

package require tcltest

namespace import ::tcltest::*

source docbuilder.tcl


test metadir-1 {Meta directory does not exist} \
    -body {
	set result [catch {processMetaDirectory testsubdir} msg]
	list $result $msg
    }                                          \
    -result {1 {Meta directory does not contain a readable 'config' file.}}

test metadir-2 {Meta directory exists, but empty config file} \
    -setup {
	makeDirectory meta
	makeFile      "" [file join meta config]
    }                                                 \
    -cleanup {
	removeFile [file join meta config]
	removeDirectory meta
    }                                               \
    -body    {
	set result [catch {processMetaDirectory meta} msg]
	list $result $msg
    }                                               \
    -result {1 {meta/config has incorrect format : must have two lines of text}} 
    

test metadir-3 {Meta directory exists and has a good file} \
    -setup {
	makeDirectory meta
	makeFile     "a b\nc d" [file join meta config]
    }                                                \
    -cleanup {
	removeFile [file join meta config]
	removeDirectory meta
    }                                              \
    -body    {
	processMetaDirectory meta
	list $::partorder $::secorder
    }                                              \
    -result  {{a b} {c d}}

test metadir-4 {Meta directory has xml files}      \
    -setup {
	makeDirectory meta
	makeFile      "a b\nc d" [file join meta config]
	makeFile      ""        [file join meta a.xml]
	makeFile      ""        [file join meta b.xml]
	makeFile      ""        [file join meta ccc.xml]
    }                                              \
    -cleanup {
	removeFile [file join meta ccc.xml]
	removeFile [file join meta b.xml]
	removeFile [file join meta a.xml]
	removeFile [file join meta config]
	removeDirectory meta
    }                                              \
    -body {
	processMetaDirectory meta
	lsort $::metafiles
    }                                              \
    -result {meta/a.xml meta/b.xml meta/ccc.xml}

cleanupTests