

package provide versionUtils 1.0


namespace eval versionUtils {
  # Define the regular expression that defines the allowed format
  # note the use of { } instead of ( ) for grouping because we
  # do not need back references
  # This should work but for some reason it won't let me have access to all
  # of the submatches. So I am forced to punt and deal with them independently
  variable fullFormat {^\d+$|^\d+\.\d+$|^\d+\.\d+-(pre\d+|alpha|beta|rc\d+|\d+){1}$}
  variable format0 {^(\d+)$}
  variable format1 {^(\d+)\.(\d+)$}
  variable format2 {^(\d+)\.(\d+)-(pre\d+|alpha|beta|rc\d+|\d+){1}$}
  variable patchFormat {^(pre\d+|alpha|beta|rc\d+|\d+){1}$}
}

##
# ::versionUtils::validFormat
#
# Checks that a certain string has the form
# x, x.y, or x.y-z, where x and y are integers 
# and z is any combination of alphanumeric letters.
proc ::versionUtils::validFormat {vsnString} {
  variable fullFormat 
  return [regexp $fullFormat $vsnString]
}

## 
# parseVersion
#
# Provided a version string of the form x.y or x.y-z
# where x and y are expected to be valid numeric
# values and z is some string of the form 

proc ::versionUtils::parseVersion {vsnString} {
  variable format0
  variable format1
  variable format2

  if {![::versionUtils::validFormat $vsnString]} {
    return -code 1 \
    "::versionUtils::parseVersion passed a version string with invalid format. String was '$vsnString'"
  }

  set parsedVersion [list]
  if {[regexp $format2 $vsnString whole major minor patch]} {
    # this is x.y-z format
    lappend parsedVersion $major
    lappend parsedVersion $minor
    lappend parsedVersion $patch
  } elseif {[regexp $format1 $vsnString whole major minor patch]} {
    # this is x.y format
    lappend parsedVersion $major
    lappend parsedVersion $minor
    lappend parsedVersion 0
  } elseif {[regexp $format0 $vsnString whole major minor patch]} {
    # this is x format
    lappend parsedVersion $major
    lappend parsedVersion 0 
    lappend parsedVersion 0
  } else {
    # Store the major version or complain if none makes sense
    return -code 1 -errorinfo "::versionUtils::parseVersion could not parse version format" \ 
        " from \"$vsnString\""
  }

  return $parsedVersion
}

##
# ::versionUtils::lessThan
#
# Given two parsed version lists of the form {major} {minor} {patch} 
# this will compare the major, minor, patch versions. This is
# in essence just comparing the major and minor versions 
# in the intuitive way.
#
# @param lhs a 3 element list containing a parsed version ordered as
#        {major minor patch}
# @param rhs a 3 element list containing a parsed version ordered as
#        {major minor patch}
# 
# @return boolean value indicating whether the lhs major,minor,patch are 
#         less than those in rhs in an element by element fashion
#
proc ::versionUtils::lessThan {lhs rhs} {
  # Extract the relevant pieces from the version
  set lhsMajor [lindex $lhs 0]
  set lhsMinor [lindex $lhs 1]
  set lhsPatch [lindex $lhs 2]

  set rhsMajor [lindex $rhs 0]
  set rhsMinor [lindex $rhs 1]
  set rhsPatch [lindex $rhs 2]

  if {$lhsMajor==$rhsMajor} {
    if {$lhsMinor==$rhsMinor} {
      # patch versions alone determine result
      return [expr [::versionUtils::comparePatch $lhsPatch $rhsPatch] < 0]
    } else {
      # minor versions differ so they alone determine result
      return [expr {$lhsMinor} < {$rhsMinor}]
    }
  } else {
    # major versions differ so they alone determine result
    return [expr {$lhsMajor} < {$rhsMajor}]
  }
}

##
# ::versionUtils::greaterThan
#
# Given two parsed version lists of the form {major} {minor} {patch} 
# this will compare the major, minor, patch versions. This is
# in essence just comparing the major and minor versions 
# in the intuitive way.
#
# @param lhs a 3 element list containing a parsed version ordered as
#        {major minor patch}
# @param rhs a 3 element list containing a parsed version ordered as
#        {major minor patch}
# 
# @return boolean value indicating whether the lhs major,minor,patch are 
#         greater than those in rhs in an element by element fashion

#
proc ::versionUtils::greaterThan {lhs rhs} {
  if {$lhs eq $rhs} {
    return 0;               # the two are equal.
  }
  
  return [expr {![::versionUtils::lessThan $lhs $rhs]}]
  
}

##
# ::versionUtils::comparePatch 
#
# This is kind of messy but it boils down to establishing that 
# the following sequence is held.
# "alpha" < "beta" < "rc0" < "rc1" < "rc2" < ... < "000" < "001" < ...
#
proc ::versionUtils::comparePatch {lhsPatch rhsPatch} {
  
  variable patchFormat

  set lhsPatch [string trim $lhsPatch]
  set rhsPatch [string trim $rhsPatch]
  
  # Complain if the patch format doesn't fit into what we understand
  if {![regexp $patchFormat $lhsPatch]} {
    return -code 1 -errorinfo "::versionUtils::comparePatch passed invalid format for lhs arg. Offensive arg = \"$lhsPatch\""
  }

  if {![regexp $patchFormat $rhsPatch]} {
    return -code 1 -errorinfo "::versionUtils::comparePatch passed invalid format for rhs arg. Offensive arg = \"$rhsPatch\""

  }


  # Now we begin with the brute force comparisons of all possible combinations
  # Yes this is messy but there are only so many combinations...
  if { [string equal "alpha" $lhsPatch] } {
    # deal with lhs == "alpha"
    if { [string equal "alpha" $rhsPatch] } {
      # this means both patches are alpha
      return 0
    } else {
      # if lhs side is alpha and the other is not, then the lhs
      # definitely comes before the rhs
      return -1
    }
  } elseif { [string equal "beta" $lhsPatch] } {
    # deal with lhs == "beta"
    if { [string equal "alpha" $rhsPatch] } {
      # beta > alpha
      return 1
    } elseif { [string equal "beta" $rhsPatch] } {
      # beta == beta
      return 0 
    } else {
      # beta < all else
      return -1
    }
} elseif { [string match "pre*" $lhsPatch] || [string match "rc*" $lhsPatch] } {
    # deal with lhs == "rc"

    if { [string equal "alpha" $rhsPatch] } {
      # rc# > alpha
      return 1
    } elseif {[string equal "beta" $rhsPatch]} {
      # rc# > beta 
      return 1
  } elseif {[string match "pre*" $rhsPatch] || [string match "rc*" $rhsPatch]} {
      # both lhs and rhs side are rc#... compare their respective numbers
      set lVsn [string range $lhsPatch 2 end] 
      set rVsn [string range $rhsPatch 2 end]
      if {$lVsn < $rVsn} { 
        # eg. rc1 < rc2
        return -1 
      } elseif {$lVsn > $rVsn} {
        # eg. rc2 > rc1
        return 1 
      } else {
        # eg. rc1 == rc1
        return 0
      }
    } else { 
      # rhs must be neither alpha, beta, or rc# and is therefore larger
      return -1
    }
  } else {
    # deal with lhs == pure number 
    if {[string equal "alpha" $rhsPatch] } {
      # 000 > alpha
      return 1
    } elseif {[string equal "beta" $rhsPatch]} {
      # 000 > beta 
      return 1
  } elseif {[string match "pre*" $rhsPatch] || [string match "rc*" $rhsPatch]} {
      # 000 > rc<infinity>
      return 1
    } else {
      # head to head battle of patch number versus patch number
      if { $lhsPatch < $rhsPatch } {
        return -1
      } elseif { $lhsPatch == $rhsPatch} {
        return 0
      } else {
        return 1
      }
    }
  }
}
