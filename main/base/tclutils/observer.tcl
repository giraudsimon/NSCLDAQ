#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2005.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#             Ron Fox
#	     NSCL
#	     Michigan State University
#

##
#  @file observer.tcl
#  @brief snit observer pattern component.
#


package provide  Observer 1.0

##
# This snit object provides generic support for the observer pattern
# within snit objects. There are two use cases:
# - One observer type per object.
# - Multiple observer types per object.
#
#  Regardless, the first thing a client must do is install 
#  one or more instances of this type as component(s).
#
# If you are only installing a single observer, you can
# normally 
#<verbatim>
#    delegate addObserver    to <observer-component>
#    delegate removeObserver to <observer-component>
#</endverbatim>
#
# Then clients of _your_ type can just use addObserver and removeObserver directly.
# If you require multiple observer types you can again install each as a component
# and:
#
# \verbatim
#    delegate addObserver to <observer-component1> as add<someTypeObserver>
#    delegate removeObserver to <observer-component1> as remove<someTypeObserver>
#
#    delegate addObserver to <observer-component2> as add<someOtherTypeObserver>
#    delegate removeObserver to <observer-component2> as remove<someOtherTypeObserver>
#
#      ...
#
# \endverbatim
#
# METHODS:
#    addObserver    - adds an observer to the ordered list of observers.
#    removeObserver - Removes an observer from the ordered list of observers.
#    invoke         - Invokes the observers in registration order.
#
snit::type Observer {
    variable observerList [list]; # List of observers.

    ##
    # addObsever 
    #
    #   Add an observer to the end of the list.
    #
    # @param observer - Command prefix  of script to invoke.
    #
    method addObserver observer {
	lappend observerList $observer
    }
    ##
    # removeObserver
    #
    #  Remove an observer from the list.  It is an error to remove
    #  a nonexistent observer.
    #
    #  @param observer - command prefix of script to remove.
    #
    method removeObserver observer {
	set index [lsearch -exact $observerList $observer]

	if {$index == -1} {
	    error "$observer is not in the observer list"
	} else {
	    set observerList [lreplace $observerList $index $index]
	}
    }
    ##
    # invoke
    #
    # Invoke the observers in the list.
    #
    # @param args - The args are appended as command words for the
    #               command.
    #
    method invoke args {
	foreach observer $observerList {
	    uplevel {*}$observer {*}$args
	}
    }
    
    
}