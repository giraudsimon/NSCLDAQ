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
# @file sshPrompt.tcl
# @brief Prompter for parameters for the SSHPipe provder.
# @author Ron Fox <fox@nscl.msu.edu>

package provide SSHPipe_Prompter 1.0

##
#  The prompter is going to get defaults from a set of configuration parameters
#  which have some defaults as well as environment variables as follows:
#
#   | variable          | Env variable   |  Default  |  Meaning               |
#   ------------------- | -------------- | --------- | ---------------------- |
#   | SSHPipeHost       | DAQHOST        | localhost | Host readout runs on   |
#   | SSHProgram        | RDOFILE        | ""        | Readout executable     |
#
#
#  These definitions are consistent with the 10.x definitions.
#
package require SSHPipe_Provider
package require Configuration
package require Tk
package require DataSourceUI
package require snit
package require dialogwrapper
package require textprompter

# Make sure the provider namespace has been established:

namespace eval ::SSHPipe {}

##
# ::SSHPipe::setParamDefaults
#
#   Sets the default parameter value.  See the table above.
#
proc ::SSHPipe::setParamDefaults {} {
    Configuration::Set SSHPipeHost "localhost"
    Configuration::Set SSHPipeProgram  ""
}
##
#  ::SSHPipe::paramEnvironmentOverrides
#
#  Apply any environment variables that override the defaults.
#
proc ::SSHPipe::paramEnvironmentOverrides {} {
    Configuration::readEnvironment SSHPipeHost DAQHOST
    Configuration::readEnvironment SSHPipeProgram  RDOFILE
}

#
#  Initialize the configuration parameters when the package is loaded:

::SSHPipe::setParamDefaults
::SSHPipe::paramEnvironmentOverrides

#----------------------------------------------------------------------------
#
#  Graphical prompting


##
# @class ::SSHPipe::ParameterPromptDialog
#
# Dialog that prompts for the parameters needed for an SSHPipe data source.
# The parameters we provide are the host, the program path and optional
# parameters passed to the program.
#
# LAYOUT:
#    +--------------------------------------------------------------------+
#    | Host:   [ entry widget ]                                           |
#    | Readout:[ entry widget ]  <Browse...>                              |
#    | Working directory [entry] <Browse...> [ ] Same as Readout program. |
#    | Parameters: [ entry widget]                                        |
#    +--------------------------------------------------------------------+
#    | <Ok>   <Cancel>                                                    |
#    +--------------------------------------------------------------------+
#
# OPTIONS:
#   -host    - Name of the host the program runs on.
#   -program - Path to the readout program that is run on the ssh pipe.
#   -parameters - Optional parameters
#   -defaultdir - Default working directory.
#
# METHODS
#   modal    - Block until either OK or Cancel has been clicked (or the dialog
#              destroyed).  [delegated to a DialogWrapper object].
#
snit::widgetadaptor ::SSHPipe::ParameterPromptDialog {
    component wrapper
    component form
    
    option -host       
    option -program    
    option -parameters
    option -defaultdir -default ""
    
    variable sameAsProgram 1
    
    delegate method modal to wrapper
    
    ##
    # constructor
    #   *  Install a top level as the hull.
    #   *  set default values for -host, -program  -parameters.
    #   *  Install a DialogWrapper as wrapper
    #   *  Ask the wrapper for the frame in which we build the prompt elements
    #   *  Build and layout the prompt areas
    #   *  Process configuration options.
    #
    # @param args - Configuration options.
    #
    constructor args {
        installhull using toplevel
        set options(-host)    [Configuration::get SSHPipeHost]
        set options(-program) [Configuration::get SSHPipeProgram]
        set options(-parameters) ""
        set options(-defaultdir) ""
        
        install wrapper using DialogWrapper $win.wrapper
        install form using $wrapper controlarea
        set f $form
        
        #ttk::label $f.hostlabel  -text {Host name:}
        #ttk::entry $f.host       -textvariable [myvar options(-host)]
        textprompt $f.host -text {Host name:} -textvariable [myvar options(-host)]
        
        #ttk::label  $f.programlabel -text {Readout program:}
        #ttk::entry  $f.program      -textvariable [myvar options(-program)]
        textprompt $f.program -text {Readout program:} \
            -textvariable [myvar options(-program)]
        ttk::button $f.findprogram  -text Browse... -command [mymethod _browseProgram]
        
        #ttk::label $f.dirlabel -text {Working directory}
        #ttk::entry $f.dir     -textvariable  [myvar options(-defaultdir)] \
        #    -state disabled
        textprompt $f.dir  -text {Working directory} \
            -textvariable  [myvar options(-defaultdir)] \
            -state disabled
        ttk::button $f.dirbrowse -text Browse... -command [mymethod _browseWD]
        ttk::checkbutton $f.samedir -text {Same as Readout} -variable [myvar sameAsProgram] \
            -onvalue 1 -offvalue 0 -command [mymethod _sameDirButton]
        
        #ttk::label  $f.paramlabel   -text {Command line options}
        #ttk::entry  $f.params       -textvariable [myvar options(-parameters)]
        textprompt $f.params -text {Command line options} \
            -textvariable [myvar options(-parameters)]
        
        grid $f.host
        grid $f.program $f.findprogram
        grid $f.dir $f.dirbrowse $f.samedir
        grid $f.params
        
        pack $wrapper
        
        # Set <Return>, and <FocusOut> events on $f.program so that
        # if the sameAsProgram variable is true the directory
        # of the program prpagates into the directory path.
        #
        bind $f.program <Return> [mymethod _programChanged]
        bind $f.program <FocusOut> [mymethod _programChanged]
        $self _sameDirButton;    # Set initial state properly.
        
        # Configure
        
        $self configurelist $args
        
    }
    #--------------------------------------------------------------------------
    # Private methods.
    #
    
    ##
    # _sameDirButton
    #   Invoked when the checkbutton changes that
    #   selects the wd be the same as the program
    #   * If the state is same (1), _programChanged is invoked
    #     to update the state and the widget is left in the
    #     disabled state.
    #   * If the state is notsame (0) the state f the entry widget
    #     is left as normal.
    #
    method _sameDirButton {} {
        if {$sameAsProgram} {
            $self _programChanged
            set state disabled
        } else {
            set state normal
        }
            $form.dir configure -state $state
            $form.dirbrowse configure -state $state

    }
    ##
    # _browseWD
    #
    #   Browse for a directory to use as the cwd.
    #   The result is set in the $form.dir field.
    #   That field is assumed to  not be disbled because the browse button
    #   -state is coupled to that of the entry.
    #
    method _browseWD      {} {
        #
        #  Initial dir is by priority
        #  - The current value.
        #  - The program's directory if it has one.
        #  - The current working directory if not
        
        set programPath [$form.program get]
        if {[$form.dir get] ne ""} {
            set initialdir [file normalize [$form.dir get]]
        } elseif {[$form.program get] ne ""} {
            set initialdir [file dirname [$form.program get]]
        } else {
            set initialdir [pwd]
        }
        
        set wd [tk_chooseDirectory -initialdir $initialdir -mustexist true \
            -parent $win -title "Choose working directory"]
        if {$wd ne ""} {
            $form.dir delete 0 end
            $form.dir insert end $wd
        }
    }
    ##
    # _programChanged
    #
    # Called when the readout program has functionally changed.
    # if $sameAsProgram is true, the directory path to the program
    # is set as the working directory.
    # Note that regardless if no directory has been chosen yet the program's
    # dir is set.
    # 
    method _programChanged {} {
        if {$sameAsProgram || ([$form.dir get] eq "") } {
            set state [$form.dir cget -state]
            $form.dir configure -state normal
            
            set program [string trim [$form.program get]]
            if {$program ne ""} {
                set wd [file dirname $program]
                $form.dir delete 0 end
                $form.dir insert end $wd
            }
            $form.dir configure -state $state
        }
    }
    
    ##
    # _browseProgram
    #    Use tk_getSaveFile to select a readout program.
    #
    method _browseProgram {} {
        set filename [tk_getOpenFile -title {Choose Readout} -parent $win \
            -defaultextension "" -filetypes [list \
                {{Exectuable files} *}            \
                {{Shell scripts}   .sh}           \
                {{Bash scripts}    .bash}         \
                {{Tcl scripts}     .tcl}          \
            ]]
        if {$filename ne ""} {
            set f [$wrapper controlarea]
            $f.program delete 0 end
            $f.program insert end $filename
            $self _programChanged;          # If needed update directory
        }
    }
}
##
# ::SSHPipe::promptParameters
#
#   Prompt for the parameters and turn them into the dict that
#   DataSourceUI::getParameters normally returns.
#
# @return dict Keys are parameter names, values are three element lists of
#              long prompt, dummy widget name, parameter value.
#
proc ::SSHPipe::promptParameters {} {
    ::SSHPipe::ParameterPromptDialog .sshpipeprompt
    
    set action [.sshpipeprompt modal]
    if {$action eq "Ok"} {
        set result [::SSHPipe::parameters]
        array set optionlookup [list host -host path -program \
            parameters -parameters wdir -defaultdir]
        dict for {key value} $result {
          set val [.sshpipeprompt cget $optionlookup($key)]
            dict lappend result $key [list] [string trim $val]
        }
        
    } else {
        set result ""
        
    }
    destroy .sshpipeprompt
    return $result
}
