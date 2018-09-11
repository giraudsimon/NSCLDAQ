#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file state.tcl
# @brief State manager for save/restore arbitrary states.
# @author Ron Fox <fox@nscl.msu.edu>

package provide StateManager 1.0
package require snit

##
# @class StateManager
#
#  Provides a mechanism to allow packages in a program to save and restore
#  internal state.  This is done by associating state variables with
#  getters and setters and providing a changed method to indicate
#  when to write a new copy of the saved file.
#
#  Save files are simply Tcl scripts that contain a bunch of set commands.
#  Those commands will be executed in a safe interpreter.
#
# OPTIONS
#   -file  - Path to the file that will hold the save script.
#
# METHOD
#   addStateVariable   - Adds a new state variable to state manager
#   listStateVariables - lists the state variables in the state manager.
#   removeStateVariable - Removes a state variable from the registered list.
#   save               - Save the state (-file must be defined by now).
#   restore            - restore the state (-file must be defined by now).
#
snit::type StateManager {
    option -file
    
    
    # State variables are an ordered list so that
    # it is possible to define the order in which data are written to file
    # and restored from the file to internal state.
    #
    #  Each element of the list is a three element list containing:
    #  *  The variable name.
    #  *  A command that will provide the correct value for that variable.  This
    #     is parameterized by the variable name when called.
    #  * A command that will react to a restored value for this variable.
    #    This command is parameterized by the variable name and new value.
    #
    variable registeredVariables [list]
    
    ##
    # constructor
    #   Contstruct the object.  Simply process the configuration options.
    # @param args  option/value pairs.
    #
    constructor args {
        $self configurelist $args
    }
    ##
    # addStateVariable
    #   adds a stateVariable to the list
    #
    # @param name   - Name of the state variable.
    # @param getter - Command to get the variable value.
    # @param setter - Command to set the variable value.
    #
    method addStateVariable {name getter setter} {
        if {[lsearch -exact -index 0 $registeredVariables $name] != -1} {
            error "There is already a registration for the variable '$name'"
        }
        lappend registeredVariables [list $name $getter $setter]
    }
    ##
    # listStateVariables
    #
    # @return the current list of state variables.  This is a list whose
    #         elements are triples of name, getter, setter.
    #
    method listStateVariables {} {
        return $registeredVariables
    }
    ##
    # save
    #   Saves the configuration.
    #   Note saves the file in #$options(-filename)#
    #   Only if the save totally works, do we move the new file on top of
    #   any old file.
    #   * -file must  have been configured.
    #   * The registered variables are itereated over and the getter
    #     for each is called.
    #   * A set command for the registered variable is written to the
    #    specified -file
    #
    method save {} {
        if {$options(-file) eq ""} {
            error {Cannot save prior to configuring -file}
        }
        
        # Figure out the temp file name:
        
        set path [file dirname $options(-file)]
        set name [file tail $options(-file)]
        set tempName [file join $path #$name#]
        
        set status [catch {
            set fd [open $tempName w]
            
            # Now iterate over the variables, getting values and writing
            # set commands.
            
            foreach variable $registeredVariables {
                
                set varname [lindex $variable 0]
                set getter  [lindex $variable 1]
                set value [{*}$getter $varname]
                puts $fd [list set $varname $value]
            }
            
            #  Close the file
            
            close $fd
        } msg]
        if {$status} {
            tk_messageBox -icon warning -type ok -title {settings file save failed} \
                -message "Could not save $options(-file) prior file is unchanged $msg"
        } else {
            file rename -force $tempName $options(-file)
        }
    }
    ##
    # restore
    #   Restores the configuratino
    #   * -file must have been configured.
    #   * A safe interpreter is created and [source] exposed
    #   * The -file is sourced into the interpreter.
    #   * For each variable in the registered list, if that variable
    #     exists in the slave interpreter, that variable's setter is called.
    #
    method restore {} {
        if {$options(-file) eq "" } {
            error {Cannot restore prior to configuring -file}
        }
        if {![file readable $options(-file)]} {
            error "The restore file '$options(-file)' does not exist or is not readable"
        }
        interp create -safe StateManagerInterp

        # Propagate the environment into the interp:
        

        foreach varname [array names ::env] {
            StateManagerInterp eval set ::env($varname) "{$::env($varname)}"
        }

        StateManagerInterp expose source
        StateManagerInterp eval source $options(-file)
                
        foreach variable $registeredVariables {
            set varname [lindex $variable 0]
            set setter  [lindex $variable 2]
            
            if {[StateManagerInterp eval info vars $varname] eq $varname} {
                set value [StateManagerInterp eval set $varname]
                {*}$setter $varname $value
            }
        }
        interp delete StateManagerInterp
    }
}

##
# @class StateManagerSingleton
#
#   This is provided for applications that need a single state saver.
#
snit::type StateManagerSingleton {
    component instance
    
    delegate option * to instance
    delegate method * to instance
    
    typevariable theInstance ""
    
    ##
    # constructor
    #    If this is the first construction, create the instance.
    #    Regardless, install the instance as the instance component.
    #    all options and methods are  delegated to the instance component
    #    so this will appear exactly like a state manager oject.
    #
    # @param args - configuration options.
    #
    constructor args {
        if {$theInstance eq ""} {
            set theInstance [StateManager %AUTO%]
        }
        install instance using set theInstance
        $self configurelist $args
    }
    
}
