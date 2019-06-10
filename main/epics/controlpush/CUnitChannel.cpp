/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <config.h>
#include "CUnitChannel.h"


using namespace std;



/**
 * Contruct a units channel.
 * we just construct our base class, and buid a simple channel into
 * m_pUnits
 */
CUnitChannel::CUnitChannel(string name) :
  CChannel(name),
  m_pUnits(0)
{
  string units = name + ".EGU";
  m_pUnits = new CChannel(units);
}
/**
 * Destruction requires that we delete m_pUnits:
 */
CUnitChannel::~CUnitChannel()
{
  delete m_pUnits;
  m_pUnits = (CChannel*)NULL;
}

/**
 * Return the name of the units channel:
 */
string
CUnitChannel::getUnitsName() const
{
  return m_pUnits->getName();
}

/**
 * Return the units channel
 */
CChannel* 
CUnitChannel::getUnitsChannel()
{
  return m_pUnits;
}

/**
 * Connect us >and< our unit channel
 */
void
CUnitChannel::Connect()
{
  CChannel::Connect();
  m_pUnits->Connect();
}
/**
 * Returns true if the channel has units.
 * this will be a false negative if the connection event has not
 * happened (e.g. because nobody connected the channel).
 */
bool
CUnitChannel::hasUnits() const
{
  return m_pUnits->isConnected();
}

/**
 * Return the units for the channel or "" if no units
 * have been gotten yet.
 */
string
CUnitChannel::getUnits() const
{
  return m_pUnits->getValue();
}
