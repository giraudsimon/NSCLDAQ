
#/*
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2017.#
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Authors:
#             Ron Fox
#             Jeromy Tompkins 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#*/
#
#/** @file:  sourceprompt.tcl
# *  @brief: Prompt for data source just like the ReadouShell does.
# *
# */
package provide guisourceprompt 1.0



package require Tk
package require snit

package require DataSourceUI
package require DataSourceManager

package require SSHPipe_Provider
package require RemoteGUI_Provider
package require Delay_Provider
package require ReadoutGui



    set dataSources [DataSourcemanagerSingleton %AUTO%]
    catch {foreach p [DataSourceManager enumerateProviders] {
        $dataSources load $p
    }}
##
# getDataSource
#   Return the dict that defines a data source to the Readout GUI.
#
# @return dict - tjhe data source dict returns empty if none selected.
proc getDataSource {} {
    
    # Choose the provider type:
    
    set provider  [DataSourceUI::getProvider \
            [DataSourceManager enumerateProviders]]
    
    if {$provider eq ""} {
        return [dict create]
    }
    
    # Get the provider params.
    

    set requiredParams [$::dataSources parameters $provider]
    set parameters [DataSourceUI::getParameters $provider $requiredParams]
    set result [dict create provider $provider]
    
   
    foreach {key value} $parameters {
        set realValue [lindex $value 2]
        dict set result $key $realValue
    }
    return $result
}
##
# @class GetDataSources
#   Form that prompts for data sources.
#
# OPTIONS:
#    -sources    - current list of sources.
#    -newcommand - Handle request for a new source.
#    -donecommand - Called when there are no more sources to define.k
#
snit::widgetadaptor GetDataSources {
    component currentsources
    
    delegate option -sources to currentsources
    option -newcommand [list]
    option -donecommand [list]
    
    ##
    # constructor
    #   Glue together the interface and process any options
    #
    constructor args {
        installhull using ttk::frame
        install currentsources using ProviderList $win.plist
        ttk::button $win.new -text "New..." \
            -command [mymethod _dispatch -newcommand]
        ttk::button $win.done -text "Done"  \
            -command [mymethod _dispatch -donecommand]
        
        # Layout the widgets:
        
        grid $currentsources -columnspan 4
        grid $win.new -row 2 -column 0
        grid $win.done -row 2 -column 3
        
        $self configurelist $args
    }
    ##
    # Command dispatchers:
    #
    
    ##
    # _dispatch
    #    Dispatches a  top level script that's stored in an option
    #
    # @param opt  - Name of the option holding the script.
    #
    method _dispatch opt {
        uplevel #0 $options($opt)
    }
}

##
# newSourceCommand
#   Handler for a request for a new data source:
#
# @param the object we need to feed the procider back to.
proc newSourceCommand w {
    set newSrc [getDataSource]
    if {$newSrc ne ""} {
        set sources [$w cget -sources]
        dict set newSrc sourceid [llength $sources]
        lappend sources $newSrc
        $w configure -sources $sources
    }
}
