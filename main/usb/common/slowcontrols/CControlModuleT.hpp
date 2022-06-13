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
#include <CControlHardware.h>
#include <iostream>
using namespace std;

/*!
   Construct a module.  This involves
   initializing the base class, our contained hardware
   and invoking the contained hardware's onAttach function so that
   configuration parameters can be registered.
   \param name : std::string
      Name of the module.  Should be a unique identifier.
   \param hardware : CControlHardwareT&
       The hardware that is configured by  us.
*/
template<class Ctlr>
CControlModuleT<Ctlr>::CControlModuleT(string name, 
                    CControlHardwareT<Ctlr>* hardware) :
  XXUSB::CConfigurableObject(name),
  m_pHardware(hardware)
{
  m_pHardware->onAttach(*this);
}
/*!
   Destroy a module.  The hardware riding along with us is assumed to have
   been dynamically created.
*/
template<class Ctlr>
CControlModuleT<Ctlr>::~CControlModuleT()
{
}

/*! 
  Copy construction.  The hardware is just cloned....configuration must be redone.
*/
template<class Ctlr>
CControlModuleT<Ctlr>::CControlModuleT(const CControlModuleT<Ctlr>& rhs) :
  XXUSB::CConfigurableObject(rhs)
{
  m_pHardware = rhs.m_pHardware->clone();
  m_pHardware->onAttach(*this);
}
/*!
  Assignment, clear our configuration, destroy our hardware
and copy the new.  Note that the module must be reconfigured at this point
*/
template<class Ctlr>
CControlModuleT<Ctlr>& 
CControlModuleT<Ctlr>::operator=(const CControlModuleT<Ctlr>& rhs)
{
  if (this != &rhs) {
    this->clearConfiguration();
    XXUSB::CConfigurableObject::operator=(rhs);
    m_pHardware = rhs.m_pHardware->clone();
    m_pHardware->onAttach(*this);
  }
  return *this;
}

/*!
  Do post configuration initialiation.
*/
template<class Ctlr>
void
CControlModuleT<Ctlr>::Initialize(Ctlr& ctlr)
{

  m_pHardware->Initialize(ctlr);

}

/*!
   Update the module.  To do this we may need to acquire the
   Vmusb Interface from readout. 
   \param ctlr : CCCUSB&
      Reference to the ctlr interface.
*/
template<class Ctlr>
string
CControlModuleT<Ctlr>::Update(Ctlr& ctlr)
{
  string result =  m_pHardware->Update(ctlr);
  return result;
}
/*!
    Set a module parameter.
    \param ctlr  : CCCUSB& 
       Reference to the VME controller
    \param what : const char*
       Name of the parameter to change.
    \param value : const char*
       Textual value of parameter.
    \return std::string
    \retval Any message to return to the set command.  In general, strings that
           begin ERROR indicate an error.
*/
template<class Ctlr>
string
CControlModuleT<Ctlr>::Set(Ctlr& ctlr, const char* what, const char* value)
{
  string reply  = m_pHardware->Set(ctlr, what, value);
  return reply;
    
}
/*!
   Retrieve a value from a  module.
   \param ctlr : CCCUSB& 
      Reference to a ctlr usb controller through which the ctlr is accessedd.
   \param what : const char*
      Name of the control point that is being modified.
   \return std::string
   \retval Any message to return to the get command.  In general strings that
       begin ERROR indicate an error, and other strings will just be the value
       of the requested point.
*/
template<class Ctlr>
string
CControlModuleT<Ctlr>::Get(Ctlr& ctlr, const char* what)
{
  string reply = m_pHardware->Get(ctlr, what);;
  return reply;
}

/*----------------------------- Support for periodic monitoring --------------------*/

/**
 ** addMonitorList allows the encapsulated hardware driver to add entries to
 ** a VMUSBReadoutList (presumably the monitoring list
 ** @param vmeList  The list to add to (passed by reference so that it can be modified).
 */
template<class Ctlr>
void
CControlModuleT<Ctlr>::addMonitorList(RdoList& vmeList)
{
  m_pHardware->addMonitorList(vmeList);
}
/**
 ** process the monitor list... again delegated to the encapsluated driver.
 ** @param pData    - Pointer to the first unused byte of the data from the list.
 ** @param remaining- Number of bytes remaining in the list that have not yet been processed.
 ** @return void*
 ** @retval pointer to the first byte of data unconsumed by this driver.
 */
template<class Ctlr>
void* 
CControlModuleT<Ctlr>::processMonitorList(void* pData, size_t remaining)
{
  return m_pHardware->processMonitorList(pData, remaining);
}
/**
 * Return the data monitored by the driver.
 * @return string
 * @retval monitored data in form and content that is
 *         driver dependent.
 */
template<class Ctlr>
string 
CControlModuleT<Ctlr>::getMonitoredData()
{
  return m_pHardware->getMonitoredData();

}
