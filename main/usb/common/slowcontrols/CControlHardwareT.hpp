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

using std::string;
#include <iostream>

#ifndef TRUE
#define TRUE -1
#endif

// The canonical are basically a bunch of no-ops.. Any CControlHardwareT
// is equivalent to any other..let the derived classes sort out subtle differences.
//
template<class Ctlr>
CControlHardwareT<Ctlr>::CControlHardwareT() :
  m_pConfig(nullptr)
{
}

template<class Ctlr>
CControlHardwareT<Ctlr>::CControlHardwareT(const CControlHardwareT<Ctlr>& rhs)
  : m_pConfig(nullptr)
{
  m_pConfig = rhs.m_pConfig;  
}

template<class Ctlr>
CControlHardwareT<Ctlr>::~CControlHardwareT()
{
}

template<class Ctlr>
CControlHardwareT<Ctlr>&
CControlHardwareT<Ctlr>::operator=(const CControlHardwareT<Ctlr>& rhs)
{
  if(this != &rhs) {
    m_pConfig = rhs.m_pConfig;
  }
  return *this;
}


template<class Ctlr>
int
CControlHardwareT<Ctlr>::operator==(const CControlHardwareT<Ctlr>& rhs) const
{
  return (m_pConfig == rhs.getConfiguration());
}

template<class Ctlr>
int 
CControlHardwareT<Ctlr>::operator!=(const CControlHardwareT<Ctlr>& rhs) const
{
  return !(*this == rhs);	// In case operator== becomes meaningful.
}

/*!
  Default initialize is a no-op.
*/
template<class Ctlr>
void 
CControlHardwareT<Ctlr>::onAttach(CControlModuleT<Ctlr>& config)
{
  m_pConfig = &config;
}


/*!
  Default initialize is a no-op.
*/
template<class Ctlr>
void 
CControlHardwareT<Ctlr>::Initialize(Ctlr& vme)
{}


/*------------------------------------- Default implementations of virtual functions -*/

/**
 ** addMonitorList is supposed to add elements to a list of VME operations that will be
 ** periodically performed to monitor device status.  As many/most devices will not
 ** need to do this, the default implementation adds nothing to the list.
 ** @param vmeList - References the vmusb list to append to.  This is passed by reference
 **                  so that it can be modified by this method.
 */
template<class Ctlr>
void
CControlHardwareT<Ctlr>::addMonitorList(RdoList& vmeList)
{
}

/** processMonitorList is called whenever data is available from a monitor list.
 ** the device driver is expected to fish its data out of the buffer and return
 ** a pointer to the next unhandled part of the list.
 ** As many device drivers do not require monitor data, the default 
 ** for this method consumes no data.
 **
 ** @param pData     - Pointer to the first unconsumed chunk of data from the monitor list.
 ** @param remaining - Number of unprocessed bytes in the monitor list.
 ** @return void*
 ** @retval Pointer to the next unconsumed byte in the list after the data we processed.
 */ 
template<class Ctlr>
void* 
CControlHardwareT<Ctlr>::processMonitorList(void* pData, size_t remaining)
{
  return pData;
}
/**
 ** Return the most recent data that is being monitored.  It's up to the
 ** concrete implementation to figure out how to represent that data.
 ** a most likely representation is a well formatted Tcl List of some sort.
 ** This implementation is provided to ensure that no existing drivers are invalidated
 ** by adding the monitor interface.
 */
template<class Ctlr>
string
CControlHardwareT<Ctlr>::getMonitoredData()
{
  string result;
  return result;
}
