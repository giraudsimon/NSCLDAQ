#
#   This program build docbook documentation from 
#   fragments stored in various directories.
#   It makes use of the following metamarkup:
#
#  <!-- chapter part -->
#  <!-- /chapter -->
#
#    The XML between these two tags are added to the
# Tutorial parts of the document in the part specified by he 
# <!-- chapter... --> tag.
#
# example:
#  <!-- chapter library -->
#  <chapter>
#     <title>Threading and synchronization</title>
#      <para>
#      ...
#  </chapter>
#
# <!-- /chapter -->
#
#  The docbook chapter above is added to the 'library' part.
#
#  <!-- manpage section -->:
#  <!-- /manpage -->
#
#  All XML that is not in either of these tags is 
#  added to chapter titled
#   "Introduction" 
#
#
#  The xml between these two tags is added to the
#  manpage section described in the section parameter of the <manpage> tag.
#
#
#  Data structures:
#    parts(name)    - Contains the contents of part 'name'.  Note that
#                     the 'Introduction' part is a part just like any other
#                     and must be specified in the part order list to
#                     be output.
#    sections(name) - Contains the manpage section contents for 'name'.
#    metafiles      - List of .xml files in the meta directory.
#
#   partorder       - A Tcl list that contains the order in which the parts should
#                     be dumped to the output file.
#   secorder        - A Tcl list that contains the order in which the sections of the
#                     reference should be dumpe dto the output file.
#
#
# Usage:
#  docbuilder.tcl metadir title topdir1 topdir2...
#
#  metadir - contains files named like;
#     config          - Contains two lines of text. The first the list of 
#                       parts that define the part ordering.
#                       The second the list of manpage sections that define
#                       the manpage ordering.
#     part_name.xml   - Provides introductory text that will be bracked by 
#                     <partintro></partintro> tags for the part 'name'.
#     refpart.xml     -Provides the partintro for the Reference part.
#     refsec_name.xml - Provides introductory material that will be put
#                       af the beginning of the reference section 'name'.
#
# topdir - Is a list of top level directories of subtrees that contain documentation.
#          Each subtree is recursed in order and *.xml for each directory is
#          processed to create the docbook.
#
# title - Is the title of the book being created.



#------------------------------------------------------------
#
#  Process the meta directory.  The config file is read into 
#  the partorder and secorder (it's an error for that file nto
#  to exist.
#
#  All the xml files are captured into the metafiles list.
#  Meta files need not exist.
#
# Parameters:
#   directory   - The top of the directory tree.
# Returns:
#    nothing
# Side-Effects:
#    sets ::partorder and ::secorder.
#
proc processMetaDirectory {directory} {

# Check config file readability.

  set configFile [file join $directory config]
  if {![file readable $configFile]} {
    error "Meta directory does not contain a readable 'config' file."
  }

  # Open the config file read it and split into a list of lines.

  set configFd [open $configFile r]
  set configData [read -nonewline $configFd]
  close $configFd

  set configData [split $configData "\n"]

  # there must be 2 lines:

  if {[llength $configData] != 2} {
    error "$configFile has incorrect format : must have two lines of text"
  }

  # Set the order variables.

  set ::partorder [lindex $configData 0]
  set ::secorder  [lindex $configData 1]

  set ::metafiles [glob -nocomplain [file join $directory *.xml]]

} 
##
# isCommentTag
#   Returns true if a line is a <!-- type -->
#   line
#
# @param line - line of text.
# @param type - the line type we care about.
#
proc isCommentTag {line type} {
    set listline [split $line " "]
    set tagopen [lindex $listline 0]
    set tagname [lindex $listline 1]
    set tagclose [lindex $listline 2]
    if {($tagopen eq "<!--")    &&
        ($tagname eq "$type") &&
        ($tagclose eq "-->")} {
          return 1
        }
    return 0
}
#-------------------------------------------------------------
#  Incorporate chapter text into a part.
#  Text is incorporated up to the line that starts
#  <!-- /chapter -->
#  Text is appended to parts($name)
#
# Parameter:
#   fd     - File from which the text is taken.
#   name   - Part name to which the text is added.
# Side Effects:
#   parts($name) is modified.
# NOTE:
#   If the end of file is seen prior to the closing tag,
#   we just take what we have with no message.
#
proc incorporateChapter {fd name} {
    while {![eof $fd]} {
      set line [gets $fd]
      if {[isCommentTag $line /chapter]} return
      append ::parts($name) $line "\n"
    }
}

#-------------------------------------------------------------
# Incorporate manpage text.  the text in the file up to the
# line that contains <!-- /manpage -->
# is inserted in sections($section).
# The <!-- /manpage --> line is supressed.
# 
# Parameters:
#   fd      - File descriptor open on the fragment file.
#   section - name of the section to append to.
# NOTE:
#   Lack of a closing tag prior to the end of the file is silently ignored.
#Side Effects:
#  The ::sections array is modified.
#
#
proc incorporateManpage {fd section} {
    while {![eof $fd]} {
      set line [gets $fd]
      if {[isCommentTag $line /manpage]} return
      append ::sections($section) $line "\n"
    }
}

#--------------------------------------------------------------
# Incorporate an xml file into the document.
# There are basically three states:
# - Outside  lines are added to the Introduction part.
# - Inside chapter  lines are added to the appropriate part in the
#   <!-- chapter .. > tag until  a <!-- /chapter --> is seen.
# - Inside manpage; lines are added to the appropriate
#   manpage section until a <!-- /manpage --> is seen.
#
#  Tags are treated as space separated lists where the first list element is
#   <!--
#  The second list element the tag name or /tag name
#  The third element is either a paramete (for tag opening) or --> for
#  closing tags.
#
# Parameters:
#   xmlFile  - Name of the XML file to incorporate
#              NOTE: If the file is not readable, nothing happens...silently.
# Returns:
#   nothing
# Side Effects:
#   the ::parts and ::sections array elements are modified.
#
proc incorporateFragment xmlFile {
    puts stderr "Incorporating $xmlFile"
    if {![file readable $xmlFile]} {
	puts stderr "Oops can't read $xmlFile"
	return
    }

    set fd [open $xmlFile r]
    
    # This loop handles the 'Outside' state and dispatches
    # to inChapter and inManpage for
    # those states.
    #

    while {![eof $fd]} {
	set line [gets $fd]
	set listline [split $line " "]
	set opening [lindex $listline 0]
	set tag     [lindex $listline 1]
	set tagarg  [lindex $listline 2]
	set tagclose [lindex $listline 3]
	if {($opening eq "<!--")    &&
	    ($tag     eq "chapter") &&
	    ($tagclose eq "-->")} {

	    # Enter chapter mode:
	    incorporateChapter $fd $tagarg

	} elseif {($opening eq "<!--")     &&
		  ($tag     eq "manpage")  &&
		  ($tagclose eq "-->")} {
	    # Enter manpage:

	    incorporateManpage $fd $tagarg
	} else {
	    # Outside everything:

	    append ::parts(Introduction) $line "\n"
	}
    }
}

#---------------------------------------------------------------
# 
#  Process the directory tree.  We recurse the directory tree
#  looking for .xml files as well as subdirs.  The
#  recursion is breadth first.
#
# Parameters:
#   directory   - Top of the directory tree to walk.
# Returns:
#   Nothing useful
# Side-Effects:
# 
proc walkDirectory {directory} {

  processDirectory $directory

  set subDirs  [glob -nocomplain -types d [file join $directory *]]

  # Need to remove . and .. from the list since we're already
  # 

  set dot [lsearch -exact $subDirs .]
  set subDirs [lreplace $subDirs $dot $dot]
  set dotdot [lsearch -exact $subDirs ..]
  set subDirs [lreplace  $subDirs  $dotdot $dotdot]

  foreach dir [lsort $subDirs] {
    walkDirectory $dir
  }
}

proc processDirectory {directory} {
  set xmlFiles [glob -nocomplain [file join $directory *.xml]]


  foreach xml [lsort $xmlFiles] {
    incorporateFragment $xml
  }
}

#-----------------------------------------------------------------
#
#  Dump the docbook header.
#
#
proc dumpHeader {} {
    puts $::outputChannel {<?xml version="1.0" encoding="UTF-8"?>}
    puts $::outputChannel {<!DOCTYPE book PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN"}
    puts $::outputChannel {"file:///usr/share/xml/docbook/schema/dtd/4.5/docbookx.dtd"}
    puts $::outputChannel { [] }
    puts $::outputChannel {>}
    puts $::outputChannel {<book>}
    puts $::outputChannel "<title>$::booktitle</title>"
}


#----------------------------------------------------------------
#  Dump the docbook trailer.
#
proc dumpTrailer {} {
    puts $::outputChannel {</book>}
}

#-----------------------------------------------------------------
#
#  Dump a part.
#   The part is bracketd by:
#
#  <part>
#     <title>Partname</title>
#  
# and
#   </part>
#
#   If there's a part_name.xml in the config directory,
#   it will be put just after the </title>
#   bracketed by 
#   <partintro>
#   </partintro>
#
# Parameter:
#   name   - Name of the part to dump 
# Implicit inputs:
#    $configdir/part_$name.xml - part header text.
#    parts($name)  what lives in the part.
# NOTE:
#   If there is no  body to
#   parts($name), then the part is omitted completely as 
#   docbook does not allow a completely empty part.
#
# 
proc dumpPart name {

    # Get the prefix file and the part contents...

    set prefixFile [lsearch -glob -inline $::metafiles */part_$name.xml]
    set contents ""
    if {[array names ::parts $name] ne ""} {
	set contents $::parts($name)
    }
    # if the prefix file does not exist and the contents of the part are
    # emtpy give up on the part.

    if {($contents eq "")} {
	return
    }

    # Part header:

    puts $::outputChannel "<part>"
    puts $::outputChannel "<title>$name</title>"

    # Part prefix (if it exists)

    if {$prefixFile ne ""} {
	set pf [open $prefixFile r]
	set prefix [read $pf]
	close $pf
	puts $::outputChannel "<partintro>"
	puts $::outputChannel $prefix
	puts $::outputChannel "</partintro>"
    }
    # Part body:

    puts $::outputChannel $contents
    
    puts $::outputChannel </part>
}
#-----------------------------------------------------------------
#
#  The reference material all lives in a separate part
#  This dumps the header for that part.
#  The part may have a partintro section if refpart.xml exists
#  in the user's config directory for the document.
#

proc refpartStart {} {
    puts $::outputChannel "<part>"
    puts $::outputChannel "<title>Reference Pages</title>"
    
    set refIntroFile [lsearch -glob -inline $::metafiles */refpart.xml]
    if {$refIntroFile ne ""} {
	set pf [open $refIntroFile r]
	set intro [read $pf]
	close $pf
	puts $::outputChannel "<partintro>"
	puts $::outputChannel $intro
	puts $::outputChannel "</partintro>"
    }

}

#-----------------------------------------------------------------
#
#  End matter for the reference part.
#
proc refpartEnd {} {
    puts $::outputChannel "</part>"
}

#-----------------------------------------------------------------
#
#  Dump a reference part to the document.
#  A reference part is actualy a <reference> section.
#  The title is the name of the section.  If 
#  the config dir has a refsec_$name.xml file it is inserted
#  bracketed by <partintro></partintro> 
#
# All this really strongly implies that stuff in this section
# be <refentry></refentry> material to conform to the document model.
#
# Parameters: 
#   sect   - The section name.
# Implicit Inputs:
#    configdir/refsect_name.xml  - part intro stuff for the reference section.
#    sections(name)              - body of the section.
#
# NOTE:
#   The entire section is omitted if the sections(name) is empty as
#   that would violate the document model.
#
proc dumpRefsec sect {

    # Don't output the reference if there's no body:

    if {[array name :::sections $sect] eq ""} {
	return
    }
    puts $::outputChannel "<reference id='man-$sect'>"
    puts $::outputChannel "<title>$sect</title>"

    # If there's an intro file, output it:

    set introFile [lsearch -glob -inline $::metafiles */refsec_$sect.xml]
    if {$introFile ne ""} {
	puts $::outputChannel "<partintro>"
	set fd [open $introFile r]
	puts $::outputChannel [gets $fd]
	close $fd
	puts $::outputChannel "</partintro>"
    }
    # Now put the body of the section:

    puts $::outputChannel $::sections($sect)

    # And close the section body with the final end tag.

    puts $::outputChannel "</reference>"
}

#-----------------------------------------------------------------
#
# Dump the document to stdout (often either redirected or piped
# elsewhere
#
proc createDocument {} {
    dumpHeader


    # If there are any parts dump them:

    if {[info globals parts] ne ""} {

	foreach part $::partorder {
	    dumpPart $part
	}
    }

    # If there are any reference sections, dump them.

    if {[info globals sections] ne ""} {
	
	refpartStart
	
	foreach sect $::secorder {
	    dumpRefsec $sect
	}
	refpartEnd
    }

    dumpTrailer

}


proc createSectionDumps {} {


# If there are any reference sections, dump them.

  if {[info globals sections] ne ""} {


    foreach sect $::secorder {
      if {[catch {set ::outputChannel [open "$sect.xml" w]} msg]} {
        puts $msg
      }
      dumpHeader
      refpartStart
      dumpRefsec $sect
      refpartEnd
      dumpTrailer
      flush $::outputChannel
      if {[catch {close $::outputChannel} msg]} {
        puts stdout $msg
      }
    }

    set outputChannel stdout
  }


}




#-----------------------------------------------------------
#  Output the program usage:
#
proc usage {} {
    puts stderr "Usage:"
    puts stderr "   docbuilder.tcl configdir title topdir1 ..."
    exit -1
}



#------------------------------------------------------------
#
#  Programe entry point.
#

# Enclosing in this block allows unit testing.

if {[info global testing] eq ""} {

  if {[llength $argv] < 3} {
    usage
  }

  set metadir       [lindex $argv 0]
  set ::booktitle   [lindex $argv 1]
  set xmldirs       [lrange $argv 2 end]

  processMetaDirectory $metadir

  # Collect the fragments:

  foreach dir $xmldirs {
#    walkDirectory $dir
    processDirectory $dir
  }

  # Dump the document:

  set ::outputChannel stdout
  createDocument
  createSectionDumps
}
