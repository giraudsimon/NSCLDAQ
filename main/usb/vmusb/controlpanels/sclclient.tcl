#
#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#     Author:
#      NSCL DAQ Development Team 
#	     NSCL
#	     Michigan State University
#	     East Lansing, MI 48824-1321
#
# @author Jeromy Tompkins


package provide ScalerClient 1.0
package require InstallRoot 

## @func startScalerClient host port ringhist ringname
#
# @brief Start up a scaler client in the background
#
# This provides a convenient callable for running the sclclient 
# program in the background. This is useful for prgorams that want
# to monitor the data stream through a scaler client.
#
# @param host the hostname where the tclserver is running
# @param port the port on which the tclserver is listening
# @param ringhost the name of the host where the ring to attach to lives
# @param ringname the name of the ring to attach to
#
# @return int
# @retval pid of the sclclient program started up  
#
proc startScalerClient {{host localhost} 
                        {port 30999} 
                        {ringhost localhost} 
                        {ringname $::env(USER)}} {
# set up the name of binary and arguments
  set root [InstallRoot::Where]
  set sclClientBin [list [file join $root bin sclclient]]
  lappend sclClientBin "--host=$host"
  lappend sclClientBin "--port=$port"
  lappend sclClientBin "--source=tcp://$ringhost/$ringname"
  lappend sclClientBin "&"
  return [exec {*}$sclClientBin]
}
