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

# @file  InputPipeline.tcl 
# @author Jeromy Tompkins 


package provide OfflineEVBInputPipeline 11.0
package require snit 
package require InstallRoot 
package require ring


## @brief An object encapsulating the parameters used in the input pipeline
#
# The parameters in that are controlled by this object are used in creating
# pipeline composed of :
#   cat files | unglom unglomid | frag2ring --strip | stdintoring inputring
# 
# The InputPipeline uses this for launching an input pipeline of the above type.
# 
snit::type OfflineEVBInputPipeParams {

  option -file      -default ""             ;#< A list of files
  option -unglomid  -default 0              ;#< labels items w/out body headers
  option -inputring -default "OfflineEVBIn" ;#< ring to dump data into

  ##  @brief a basic constructor
  # 
  # Configures the list
  #
  # @param args   option-value pairs
  constructor {args} {
    $self configurelist $args
  }

  ## @brief Validate the options
  #
  # Passes a list around to a number of different validation mehtods that each
  # append any error messages that they might find.
  #
  # @returns list of errors
  method validate {} {
    
    set errors [list]

    $self validateFiles errors
    $self validateUnglomId errors
    $self validateInputRing errors

    return $errors
  }


  ## @brief Valdiate the list of files provides
  #
  # Checks whether any input files were even specified and whether they exist
  # if they have been specified.
  #
  # @param errors_  variable name of list
  #
  method validateFiles {errors_} {
    upvar $errors_ errors

    # check for presence of input files
    if {[llength $options(-file)] == 0 } {
      lappend errors "No input file(s) provided."
    }

    # check for existences of any that have been provided
    foreach file $options(-file) {
      if {![file exists $file]} {
        lappend errors "$file does not exist."
      }
    }

  }


  ## @brief Validate the unglom id
  #
  # Merely ensure that it is a non-negative integer.
  #
  # @param errors_  variable name of list
  #
  method validateUnglomId {errors_} {
    upvar $errors_ errors

    if {$options(-unglomid) < 0} {
      set msg "unglom source id option must be greater than or equal "
      append msg "to zero. User has provided $options(-unglomid)"
      lappend errors $msg
    }

  }

  ## @brief Validate the input ring
  #
  # Check that the the specified ring exists on localhost
  #
  # @param errors_  variable name of list
  #
  method validateInputRing {errors_} {
    upvar $errors_ errors

    if {! [file exists [file join /dev shm $options(-inputring)]]} {
      set msg "Ring does not exist on localhost. User has provided "
      append msg "$options(-inputring)"
      lappend errors $msg
    }

  }


  ## @brief Create a new instance with same state
  #
  # This creates a new OfflineEVBInputPipeParams whose options have the
  # same value as the original instance.
  #
  # @returns a new OfflineEVBInputPipeParams
  #
  method clone {} {
    
    # get all of the options and their values and make a dict of them
    set state [dict create]
    foreach opt [$self info options] {
      set value [$self cget $opt]
      dict set state $opt $value 
    }

    # return a new snit object with the same params
    return [OfflineEVBInputPipeParams %AUTO% {*}$state]
     
  }

}

###############################################################################
###############################################################################
###############################################################################


## @brief Constructs and manages a pipeline for unglomming a data file 
#
# This manages data stream will stream a set of data files through the 
# following pipeline:
#
#   cat file0 file1 ... | unglom --id | frag2ring --strip | stdintoring ring
#
# A consequence of running data through this pipeline is that the outermost 
# level built data is split into its building blocks and further that all of the
# data items that formerly lacked any body headers will be assigned a body
# header.
#
# This expects to receive its configuration parameters via an
# OfflineEVBInputPipeParams object. Also destruction of this will cause the
# pipeline to be destroyed.
#
snit::type OfflineEVBInputPipeline {
  
  option -daqroot -default {::InstallRoot::Where} \
                  -validatemethod SetPath   ;#< specifies the install path to
                                            ;#  use

  variable running    false   ;#< indicates whether the pipeline is running
  variable fd         -1      ;#< the file descriptor for the pipeline

  ## @brief A basic constructor
  # 
  # Configures the options from the argument
  #
  # @param  args  a list of option-value pairs
  #
  constructor {args} {
    $self configurelist $args
  }

  ## @brief Destroy the pipeline if running
  #
  destructor {
    if {$fd > 0} {
      catch "close $fd" msg
    }
  }

  ## @brief Validate the parameters that are passed into the 
  #
  # @todo Use the OfflineEVBInputPipeParams::validate method instead
  #
  # Mostly this demands that there is a certain set of options that can
  # be queried. It is not necessary that the param object passed in as
  # an argument is anything more than a snit object with the proper 
  # options defined. However, it is best to just use what was designed.
  # 
  # @param params   an OfflineEVBInputPipeParams object
  # @param context  name of proc in which this method was called
  #
  # @throws error if unglomid is not specified, inputring is not present,
  #               of file is not present.
  typemethod validateParameters {params context} { 
     set keys [$params info options]
     if {"-unglomid" ni $keys} {
       set msg "$context : no source id specified for unglom"
       return -code error $msg 
     }

     if {"-inputring" ni $keys} {
       set msg "$context : no ring specified for input pipeline"
       return -code error $msg
     }

     if {[$params cget -file] eq ""} {
       set msg "$context : no input file specified for input pipeline"
       return -code error $msg
     } else {
       foreach path [$params cget -file] {
         if {![file exists $path]} {
           set msg    "$context : input file \"$path\" specified for input "
           append msg "pipeline but does not exist"
           return -code error $msg
         }
       }
     }

     # if none of the above conditions returned early, the
     # parameters are reasonable
  }

  ## @brief ensure that a path exists before setting it
  #
  # @throws error if the path doesn't exist
  method SetPath {option value} {
    if {![file exists $value]} {
      return -code error "OfflineEVBInputPipeline::SetPath passed path that does not exist for option: $option"
    } 
  }

  ## @brief Launch the pipeline
  #
  # Given a parameter set (OfflineEVBInputPipeParams), create and configure the
  # pipeline.
  # 
  # If the ring that is to be used does not exist, this will simply create it.
  # Validation of the parameters occurs to ensure that we can abort trying to
  # create the pipeline if it is destined to fail anyway.
  #
  # @param params an OfflineEVBInputPipeParams object
  #
  method launch {params} {
    if {[$params cget -inputring] ni [ringbuffer list]} {
      # failed to find the ring
      ringbuffer create [$params cget -inputring]
    }

    # Check whether or not the params contain good infomation
    OfflineEVBInputPipeline validateParameters $params "OfflineEVBInputPipeline::launch"

    # Set up the command to run based on the namespace variables
    set cmd [$self _createPipelineCommand $params]

    # launch the pipeline 
    $self setPipeFD [open $cmd r]
    # update the running state
    $self setRunning true

    # configure the pipeline
    set theFD [$self getPipeFD]
    chan configure $theFD -blocking 0
    chan configure $theFD -buffering line 
    chan event $theFD readable [mymethod _handleReadable]

  }

  ## @brief Format the command pipeline using the parameter set
  #
  # @param params   an OfflineEVBInputPipeParams object
  #
  # @returns the command pipeline 
  method _createPipelineCommand {params} {

    set unglomid  [$params cget -unglomid]
    set inputring [$params cget -inputring]
    set fnames     [$params cget -file]

    set daqbin [file join [InstallRoot::Where] bin]
    set pipeline     "| cat "
    foreach file $fnames {
      append pipeline "$file "
    }
    append pipeline  "| $daqbin/unglom --id $unglomid "
    append pipeline  "| $daqbin/frag2ring --strip "
    append pipeline  "| $daqbin/stdintoring $inputring "
    append pipeline  "|& cat"

    return $pipeline
  }

  ## @brief Access the file descriptor
  #
  # @returns file descriptor handle
  method getPipeFD {} {
    return $fd
  }

  ## @brief Set the fd handle
  # 
  # This really is not meant to be used any more than in testing or 
  # internally to this method.
  #
  # @param newfd  the file handle
  method setPipeFD {newfd} {
    set fd $newfd
  }

  ## @brief Set the running state flag
  #
  # Set the flag to indicate whether the pipeline is running or not. Querying
  # the running state is what enable the user to determine if the pipeline
  # has exited or not.
  #
  # @param yesno  a boolean value to indicate if the state is running
  method setRunning {yesno} {
    set running $yesno
  }

  ## @brief Retrieve running status
  #
  # @returns boolean indicating whether the pipeline is active or not
  method getRunning {} {
    return $running
  }

  ## @brief Callback used when the pipeline is readable
  #
  # This manages the running status. Any messages output from the pipeline
  # are redirected to stdout as normal output. When the pipeline eof is 
  # set a message is printed and this callback is unregistered.
  # 
  method _handleReadable {} {
    set theFD [$self getPipeFD]
    set line [read $theFD]
    if {[eof $theFD]} {
      puts "Input pipeline has exited"
      chan event $theFD readable {}
      catch {close $theFD} msg

      $self setPipeFD -1
      $self setRunning false
    } else {
      puts $line
    }
  }
}



