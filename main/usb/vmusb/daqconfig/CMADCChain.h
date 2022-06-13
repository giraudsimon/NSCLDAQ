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
#ifndef __CMADCCHAIN_H
#define __CMADCCHAIN_H

#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
#endif

#ifndef __CMESYTECBASE_H
#include "CMesytecBase.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif 
#endif

#ifndef __STL_LIST
#include <list>
#ifndef __STL_LIST
#define __STL_LIST
#endif
#endif


class CReadouModule;
class CVMUSB;
class CVMUSBReadoutList;
class CMADC32;

/*!
   This class implements a chain of MADC32's to be read out via CBLT.
   The module contains an ordered list of modules which must be provided
   from 'left to right' in the VME crate.
   The list has a CBLT and MCST address assigned to it and 
   the MCST is used to re-enable after readout as well as to synchronously clear
   the timestamp counters in the modules in the chain.
   
   Configuration Parameters are:

\verbatim
   Parameter         Value type/default  Meaning
   -cbltaddress      uint32 0            Chained block transfer base address.
   -mcastaddress     uint32 0            Multicast base addresss.
   -maxwordspermodule int 1024           Maximum number of words to read from each  module
                                         this will actually be rounded up to the next  event boundary.
   -modules          list of strings {}  Names of the MADC32 modules included in the chain.
\endverbatim


*/
class CMADCChain : public CReadoutHardware
{
  // internally used data types:

private:
  typedef std::list<CMesytecBase*>    ChainList;
  typedef ChainList::iterator    ChainListIterator;
  
  // Per object data:
private:
  ChainList         m_Chain;	// List of modules first must be leftmost, last must be right most.
  CReadoutModule*   m_pConfig;  // My configuration database.


public:
  CMADCChain();
  CMADCChain(const CMADCChain& rhs);
  virtual ~CMADCChain();
  CMADCChain& operator=(const CMADCChain& rhs);

  // Disallowed canonicals:

private:
  int operator==(const CMADCChain& rhs) const;
  int operator!=(const CMADCChain& rhs) const;
public:


  // overridable oeprations on constructed objects:

public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;

  // utilities:

private:

  std::list<std::string> getModules();
  void                   namesToList(std::list<std::string> moduleNames); // populate m_Chain.
  static bool moduleChecker(std::string name, std::string value, void* arg);
  
};
#endif
