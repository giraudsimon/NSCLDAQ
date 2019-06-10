/**

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
# @file   ThreadTest.cpp
# @brief  Test CStateMonitorThread
# @author <fox@nscl.msu.edu>
*/
#include <string>
#include <iostream>
#include <CStateMonitorThread.h>
#include <unistd.h>


/**
 *  Main is just going to start thread and every second ask
 *  for the state, title, run number and recording status.
 *
 *  Requires a pair of parameters in order the request and pub/sub
 *  URIs.
 */

int main(int argc, char** argv)
{
    std::string reqUri = argv[1];
    std::string subUri = argv[2];
    
    CStateMonitorThread sm(reqUri, subUri);
    sm.start();
    
    while(1) {
        sleep(1);
        std::cout << "-----------------------\n";
        std::cout << "State:  " << sm.getState() << std::endl;
        std::cout << "TItle:  " << sm.getTitle() << std::endl;
        std::cout << "RunNum: " << sm.getRunNumber() << std::endl;
	std::string state = sm.getRecording() ? "On" : "Off";
        std::cout << "Recording: " << state << std::endl;
                                       
     }
    
    
}
