#!/usr/bin/env python


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
# @file   state.py  
# @brief  Contains the global state of the program.
# @author <fox@nscl.msu.edu>

##
# State
#   This is a class of static public members.  It holds the global
#   state of the configuration editor.  See the comments inside the class
#   for more information.
#

class State:
    # Projects are represented by Project objects that are backed up in an
    # sqlite database.  The program may have a project open or it may not.
    # If it has a project open, then the project object will be in the variable
    # below otherwise that variable will have the value None
    
    project = None
    
    
state = State()