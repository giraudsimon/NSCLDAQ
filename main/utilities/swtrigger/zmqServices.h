/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  zmqServices.h
 *  @brief: Definitions for ZMQ services that we use in the trigger framework.
 */
#ifndef ZMQ_SERVICES_H
#define ZMQ_SERVICES_H

#include <string>

// This is the URI of a push/pull that connects the data source with the
// data distributor:

const std::string DataSourceURI("inproc://DataSource");

// This is the URI of the router/dealer that connects the data distributor
// to the worker threads:

const std::string DataDistributionURI("inproc://DataDistributor");

//  This is the URI of the push pull fan-in/out that connects
//  Workers with the sorting thread:

const std::string SorterURI("inproc:://Sorter");

// This is the URI of the push/pull that connects the sorter with the output
// thread

const std::string OutputURI("inproc:://Outputter");


#endif