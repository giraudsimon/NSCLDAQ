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
# @file   CStateMonitorBaseTest.cpp
# @brief  Test the statemonitor base class.
# @author <fox@nscl.msu.edu>
*/


#include <CStateMonitor.h>
#include <iostream>

void
initDone(CStateMonitorBase* pMonitor) {
    std::cout << "Initialization complete\n";
}


class MyStateMonitor : public CStateMonitorBase
{
public:
    MyStateMonitor(std::string requestURI, std::string stateURI) :
        CStateMonitorBase(requestURI, stateURI, initDone) {}
    virtual ~MyStateMonitor() {}
    
protected:
    virtual void initialState(std::string state) {
      CStateMonitorBase::initialState(state) ;
      std::cout << "Initial state: " << state << std::endl;
    }
    virtual void transition(std::string transition) {
        std::cout << "Transition from " << getState() << " to " << transition << std::endl;
        CStateMonitorBase::transition(transition);
        std::cout << "base class transition returned\n";
    }
    virtual void runNumMsg(std::string body) {
        std::cout << "run number: " << body << std::endl;
    }
    virtual void titleMsg(std::string body) {
        std::cout << "Title: " << body << std::endl;
    }
    virtual void recordMsg(std::string body) {
        CStateMonitorBase::recordMsg(body);
        if (getRecording()) {
            std::cout << "Recording\n";
        } else {
            std::cout << "Not recording\n";
        }
    }
    
};

int main(int argc, char** argv) {
    std::string reqURI(argv[1]);
    std::string stateURI(argv[2]);
    
    MyStateMonitor monitor(reqURI, stateURI);
    
    monitor.run();
 }
