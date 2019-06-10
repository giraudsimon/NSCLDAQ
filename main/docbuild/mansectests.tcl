set testing 1


package require tcltest
namespace import ::tcltest::*

source docbuilder.tcl

set result [list testing "some text\n"]
test mansec-1 {text and then close tag}          \
    -setup {
	makeFile "some text\n<!-- /manpage -->" test.xml
    }                                            \
    -cleanup {
	unset ::sections
    }                                            \
    -body {
	set fd [open test.xml r]
	incorporateManpage $fd testing
	close $fd
	list [array names ::sections] $::sections(testing)

    }                                            \
    -result $result




cleanupTests