#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#    Jeromy Tompkins
#	   NSCL
#	   Michigan State University
#	   East Lansing, MI 48824-1321



package provide mcfd16channelnames 1.0


## @namespace MCFD16ChannelNames
#
# @brief The names of the channels that are represented as ttk::entry widgets in
# the MCFD16 Gui Controls application. This serves as a convenient way to make
# them available to multiple different widget that might control them while at
# the same time providing a simpler means for accessing them. It is a
# lighterweight alternative to providing a NameManager snit::type that manages
# notifying all entities that care about the name values about any changes. It
# is less safe in general as effectively a global variable, but in the interest
# of time and the size of the application, this was chosen. It is also
# considerably simpler.
#
namespace eval MCFD16ChannelNames {

  variable chan0 Ch0
  variable chan1 Ch1
  variable chan2 Ch2
  variable chan3 Ch3
  variable chan4 Ch4
  variable chan5 Ch5
  variable chan6 Ch6
  variable chan7 Ch7
  variable chan8 Ch8
  variable chan9 Ch9
  variable chan10 Ch10
  variable chan11 Ch11
  variable chan12 Ch12
  variable chan13 Ch13
  variable chan14 Ch14
  variable chan15 Ch15

}
