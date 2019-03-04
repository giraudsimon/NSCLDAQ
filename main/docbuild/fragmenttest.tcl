set testing 1

package require tcltest
namespace import ::tcltest::*
source docbuilder.tcl


# Replace the incorporateChapter and
# incorporateManpage procs so we can 
# can generate known stuff.
#

rename incorporateChapter old_incorporateChapter
rename incorporateManpage old_incorporateManpage


set mansec [list]
set theparts  [list]

proc incorporateChapter {fd part} {
    lappend ::theparts $part
}
proc incorporateManpage {fd section} {
    lappend ::mansec $section
}

set expected [list "hi\nthere\n\n" [list] [list]]
test fragment-1 {Test no processed tags..}           \
    -setup {
	makeFile "hi\nthere" test.xml
	set mansec [list]
	set theparts  [list]
    }                                         \
    -cleanup {
	unset ::parts
	set mansec [list]
	set theparts  [list]
	removeFile test.xml
    }                                         \
    -body {
	incorporateFragment test.xml
	list $::parts(Introduction)  $::mansec $::theparts

    }                                         \
    -result $expected


test fragment-2 {Test file with chapter tag} \
    -setup {
	makeFile "<!-- chapter george -->" test.xml
	set mansec [list]
	set theparts [list]
    }                                        \
    -cleanup {
	removeFile test.xml
	set mansec [list]
	set theparts [list]
	
    }                                        \
    -body {
	incorporateFragment test.xml
	list  $theparts $mansec
    }                                        \
    -result {george {}}


test fragment-2 {test file with manpage tag} \
    -setup {
	makeFile "<!-- manpage harry -->" test.xml
	set mansec [list]
	set theparts [list]
    }                                       \
    -cleanup {
	removeFile test.xml
	set mansec [list]
	set theparts [list]
    }                                       \
    -body {
	incorporateFragment test.xml
	list $theparts $mansec
    }                                        \
    -result {{} harry}

test fragment-3 {test file with unrecognized tag -> Intro part} \
    -setup {
	makeFile "<!-- badtag -->" test.xml
	set mansec [list]
	set theparts [list]
    }                                                              \
    -cleanup {
	removeFile test.xml
	set mansec [list]
	set theparts [list]
	unset ::parts
    }                                                            \
    -body {
	incorporateFragment test.xml
	list $::parts(Introduction) $theparts $mansec
    }                                                              \
    -result "{\n\n<!-- badtag -->\n\n} {} {}"

test fragment-4 {test file with a variety of good tags} \
    -setup {
	makeFile "<!-- chapter hoo -->\n<!-- chapter ha -->\n<!-- manpage hum -->\n" test.xml
	set mansec [list]
	set theparts [list]
    }                                                   \
    -cleanup {
	removeFile test.xml
	set mansec [list]
	set theparts [list]
    }                                                    \
    -body {
	incorporateFragment test.xml
	list $mansec $theparts
    }                                                     \
    -result {hum {hoo ha}}

rename incorporateChapter {}
rename incorporateManpage {}

rename old_incorporateChapter incorporateChapter
rename old_incorporateManpage incorporateManpage

cleanupTests