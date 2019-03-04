#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2017.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#           Jeromy Tompkins
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

package provide ReadoutGUIAppLauncher 1.0

package require snit
package require Tk
package require Wait


### \brief The view for the ReadoutGUI application launcher
#
#  This is a very thin view. It relays just about all of this logic to the 
#  presenter object (i.e. ReadoutGUIAppLauncher). You should not try to
#  instantiate one of these by itself directly. Rather you should instantiate
#  the ReadoutGUIAppLauncher and let it create it. 
#
snit::widget AppLauncherView {

	variable controller
	variable id

	constructor {theController} {
		set controller $theController
		set id 0
	}


	### \brief Place a button at a certain grid location
        # 
        # If a button already exists at the location, replace it with the new one.
	# The button will be gridded and set to stretch in all directions.
        #
        # \param button		the name of the button widget
	# \param row		the row to put it in
	# \param column		the column to put it in
	#	
	#
	method setButton {button row column} {
		set currentButton [grid slaves $win -row $row -column $column]
		if {($currentButton ne {})} {
			if {($currentButton ne $button)} {
				# there is a different button here.. get rid of it
				grid forget $currentButton	
			} else {
				# the button is already there... abort
				return
			}
		}

		# grid the button
		grid $button -row $row -column $column -sticky nsew -pady 3 -padx 3
		grid columnconfigure $win $column -weight 1
	}

	method getNumberOfButtons {} {
		return [llength [grid slaves $win]]
        }

	method getGeometry {} {
		return [grid size $win]
        }

	method createButton {label} {
		set name $win.button$id
		set btn [ttk::button $name -text $label -command [mymethod onClick $name]]
		incr id

		return $btn
	}

	method onClick {buttonName} {
		$controller onClick $buttonName
	}
	

}

### \brief ReadoutGUIAppLauncher
#
# A presenter for the AppLauncherView widget. Nearly all of the logic exists in this class.
# It is best to think of this as the actual widget and directly interact with this. 
# To illustrate how this works, the following code snippet demonstrates how to set up 
# two buttons and then add the launcher to the ReadoutGUI. It also shows how to disable
# and enable a button.
#
# \code
#
# package require ReadoutGUIAppLauncher 
#
# ReadoutGUIAppLauncher launcher .launch 4
# set btn0 [launcher.appendButton {Button 0 Text} {command for button 0}]
# set btn1 [launcher.appendButton {Button 1 Text} {command for button 1}]
# 
# launcher addToReadoutGUI
#
# # disable a button
# launcher disableButton $btn0
#
# # enable a button
# launcher enableButton $btn0
# 
# \endcode
#
snit::type ReadoutGUIAppLauncher {

	component view
	variable commandDict 
	variable nColumns

	### \brief Constructor
	#
	# \param view_name	name of the view to construct
	# \param nCols		number of columns to arrange buttons in
	#
	# The constructor will instantiate a view to control.
	#
	constructor {view_name {nCols 1}} {
		install view using AppLauncherView $view_name $self
		set commandDict [dict create]
		set nColumns $nCols
	}

	### \brief Destructor
	#
	# It destroys the view.
	#
	destructor {
		catch {destroy $view}
	}

 	### \brief adds a button to the view
	# 
	# A new button will be created with the provided text. When pressed, the commandString
        # will be executed in the background. 
        #
        # \param label		text to display on the button
	# \param commandString	the actual command and arguments that are intended to be executed
	#
	# \return name of the created button
	method appendButton {label commandString} {
		set btn [$view createButton $label]
		dict set commandDict $btn [dict create button $btn command $commandString]

		set geo [$view getGeometry]
		set nButtons [$view getNumberOfButtons]
		
		set col [expr $nButtons%$nColumns]
		set row [expr $nButtons/$nColumns]

		$view setButton $btn $row $col
		return $btn
	}

	### \brief Execute the command associated with the button
	#
	# \param buttonName the name of the button whose command should be executed
	#
	# This simply execs the command in the background
	# 
	method onClick {buttonName} {
		set command [dict get $commandDict $buttonName command]

		set process [exec {*}$command &]
	}

	### \brief Disable a specific button 
	#
	# \param buttonName the	name of the button to disable
	#
	# The name of the button is returned from the appendButton method
	method disableButton {buttonName} {
		$buttonName configure -state disabled
	}

	### \brief Enable a specific button 
	#
	# \param buttonName the	name of the button to enable
	#
	# The name of the button is returned from the appendButton method
	method enableButton {buttonName} {
		$buttonName configure -state !disabled
        }

	### \brief Display the view on the ReadoutGUI (or any other root parent)
	#
	# \param parent		the name of the view's parent widget
	#
	#
	method addViewToParentGUI {{parent .}} {
		set geo [grid size $parent]
		
		set nCols [lindex $geo 0]
		set nRows [lindex $geo 1]

		set row [expr {$nRows+1}]

		grid $view -row $row -columnspan $nCols -sticky nsew -padx 3 -pady 3 
        }

	# The following two methods exists primarily for testing
	method _getCommandDict {} {
		return $commandDict
	}

	method _setCommandDict {cmdDict} {
		set commandDict $cmdDict
	}
}

