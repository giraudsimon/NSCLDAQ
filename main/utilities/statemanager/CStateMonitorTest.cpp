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
# @file   CStateMonitorTest.cpp
# @brief  test the final full blown state monitor.
# @author <fox@nscl.msu.edu>
*/


#include "CStateMonitor.h"
#include <iostream>

void
NotReady(CStateMonitor* pMonitor, std::string from, std::string to, void* arg)
{
    const char* p =  reinterpret_cast<const char*>(arg);
    
    std::cout << "NotReady transition " << from << "->" << to << " (" << p << ")\n";
}

void
Ready(CStateMonitor* pMonitor, std::string from, std::string to, void* arg)
{
    const char* p =  reinterpret_cast<const char*>(arg);
    
    std::cout << "Ready transition " << from << "->" << to << " (" << p << ")\n";
}

void Title(CStateMonitor* pMonitor, std::string title, void * arg)
{
    std::cout << "New title: " << title << std::endl;
}
void Run(CStateMonitor* pMonitor,  int run, void* arg)
{
    std::cout<< "New run: " << run << std::endl;
}
void Record(CStateMonitor* pMonitor, bool state, void* arg)
{
    std::cout << (state ? "Recording " : "Not recording") << std::endl;
}

int main(int argc, char**argv)
{
    std::string reqURI   = argv[1];
    std::string stateURI = argv[2];
    
    CStateMonitor mon(reqURI, stateURI);

    mon.Register("NotReady", NotReady, const_cast<char*>("Some text"));
    mon.Register("Ready", Ready, const_cast<char*>("Different text"));
    mon.setTitleCallback(Title, 0);
    mon.setRunNumberCallback(Run, 0);
    mon.setRecordingCallback(Record);
    
    mon.run();
}