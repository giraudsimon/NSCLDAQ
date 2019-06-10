#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Authors:
#      NSCLDAQ Development Group
#
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321

# This software is Copyright by the Board of Trustees of Michigan
# State University (c) Copyright 2297884. 

# @file  HoistPipeline.tcl 
# @author Jeromy Tompkins 



package provide OfflineEVBHoistPipeline 11.0
package require snit

## @brief A snit type encapsulating the info for launching a ringFragmentSource
#
# The OfflineEVBHoistPipeParams is a very primitive snit::type that consists
# mostly of some state held in options. It has ability to produce a clone of 
# itself and also to validate its values. 
#
snit::type OfflineEVBHoistPipeParams {

  option -sourcering  -default "OfflineEVBIn"   ;#< ring to attach to 
  option -tstamplib   -default ""               ;#< tstamp lib
  option -id          -default [list 0]                ;#< source id 
  option -info        -default "Data from OfflineEVBIn" ;#< Info message to display 
  option -expectbheaders -default 1             ;#< Whether to use 
                                                ;#  --expectbodyheaders flag



  ## @brief Basic constructor to parse options
  #
  constructor {args} {
    $self configurelist $args
  }

  ## @brief Inspect the values of the options for 
  #
  # Passes a list around to the various validation 
  # methods for each option. Each of these methods will then append
  # any new error messages to the the list. This way, it is
  # possible to harvest all of the issues that could be wrong in one
  # shot and report them all to the user.
  #
  # @returns the list of errors
  method validate {} {
     set errors [list]

     $self validateSourceRing errors
     $self validateTstampLib errors
     $self validateId errors
     $self validateInfo errors

     return $errors

  }

  ## @brief Validate the ring buffer name
  #
  # This checks to see whether the ring specified as the option -sourcering
  # exists on localhost. If it doesn't, it appends a message.
  # 
  # @param errors_  the variable name of the list
  #
  method validateSourceRing {errors_} {
    upvar $errors_ errors
    if {! [file exists [file join /dev shm $options(-sourcering)]]} {
       lappend errors "Ring $options(-sourcering) does not exist on localhost."
    }
  }

  ## @brief Validate the tstamp library
  #
  # Ensure that the tstamp library has been specified if the -expectbheaders
  # option is false, and also ensure that it actually exists if it was 
  # specified.
  #
  # @param errors_  the variable name of the list
  #
  method validateTstampLib {errors_} {
    upvar $errors_ errors
    # check for missing tstamplib
    if {$options(-tstamplib) eq "" } {
      # this is only bad if the -exectbheaders option is specified
      if { ! $options(-expectbheaders) } {
        lappend errors "Timestamp extractor library has not been specified and is mandatory."
      }
    } elseif {! [file exists $options(-tstamplib)] } {
       lappend errors "Timestamp extractor library \"$options(-tstamplib)\" does not exist."
    }
  }


  ## @brief Validate the source id provided
  # 
  # We onlyrequire that all elements in id list are non-negative integers
  #
  # @param errors_  the variable name of the list
  #
  method validateId {errors_} {
    upvar $errors_ errors
    
    puts $options(-id)
    foreach id $options(-id) {
      if {![string is integer $id]} {
        lappend errors "Source ids for ringFragmentSource must be integers. User specified $id."
      }
      if {$id < 0} {
        lappend errors "Source ids for ringFragmentSource must be greater than or equal to 0. User specified $id."
      }
    }
  }

  ## @brief Valideate the info
  #
  # Simply make sure that it is not an empty string
  #
  # @param errors_  the variable name of the list
  #
  method validateInfo {errors_} {
    upvar $errors_ errors
    if {$options(-info) eq ""} {
      lappend errors "Source info for ringFragmentSource has not been defined."
    }
  }

  ## @brief Create new object that has all of the same option values 
  #
  # @returns a clone of this
  method clone {} {
    
    # get all of the options and their values and make a dict of them
    set state [dict create]
    foreach opt [$self info options] {
      set value [$self cget $opt]
      dict set state $opt $value 
    }

    # return a new snit object with the same params
    return [[$self info type] %AUTO% {*}$state]
     
  }

}
