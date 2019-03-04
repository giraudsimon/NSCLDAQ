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
# @file   Polltest.cpp
# @brief  Test the CZMQEventLoop class 
# @author <fox@nscl.msu.edu>
*/
#include "CStateMonitor.h"
#include <zmq.hpp>
#include <stdlib.h>
#include <iostream>



bool tick(CZMQEventLoop* pLoop)
{
    static int ticks = 0;
    std::cout << "Tick\n";
    ticks++;
    return ticks < 100;
}

void subHandler(CZMQEventLoop* pLoop, zmq::pollitem_t* pItem, void* param)
{
  zmq_msg_t message;
  zmq_msg_init(&message);

  zmq_recv(pItem->socket, &message,  0);

  size_t n = zmq_msg_size(&message);
  char msg[n+1];
  memset(msg, 0, n+1);
  memcpy(msg, zmq_msg_data(&message), n);

  std::cout << "Got: '" << msg << "\n";
  
  zmq_msg_close(&message);
}

int main(int argc, char**argv)
{
    char* pStateURI = argv[1];
    CZMQEventLoop eventLoop;
    
    // Make a SUBscription socket to the URI passed in:
    
    zmq::context_t context(1);
    zmq::socket_t  receiver(context, ZMQ_SUB);
    receiver.connect(pStateURI);
    receiver.setsockopt(ZMQ_SUBSCRIBE, "STATE:", 6);
    receiver.setsockopt(ZMQ_SUBSCRIBE, "TRANSITION:", 10);
    
    eventLoop.Register(receiver, ZMQ_POLLIN, subHandler);
    eventLoop.pollForever(1000*1000, tick);
}
