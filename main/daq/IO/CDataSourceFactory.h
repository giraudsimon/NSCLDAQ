/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef CRINGDATASOURCEFACTORY_H
#define CRINGDATASOURCEFACTORY_H

#include <string>
#include <vector>
#include <stdint.h>

/**
 * @file CDataSourceFactory.h
 * @author Ron Fox
 * @brief Define a class that produces CRingDataSource objects.
 */

class CDataSource;

/**
 * @class CDataSourceFactory
 *
 *  This class produces ring data source objects given a URI and other parameters.
 *  This centralizes the knowledge about how to interpret ring URI's and how to
 *  create the appropriate corresponding data source used by utilities that 
 *  can take datat from online and offline ring sources.
 *
 */
class CDataSourceFactory {
public:
  static CDataSource* makeSource(std::string uri, 
				     std::vector<uint16_t> sample, std::vector<uint16_t>exclude);
};

#endif
