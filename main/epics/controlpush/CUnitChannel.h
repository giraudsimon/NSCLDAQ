#ifndef CUNITCHANNEL_H
#define CUNITCHANNEL_H
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

#include "CChannel.h"


class CUnitChannel : public CChannel
{
private:
  CChannel* m_pUnits;		// Associated units.
public:
  CUnitChannel(std::string(name));
  virtual ~CUnitChannel();

  // Forbidden fruit:

private:
  CUnitChannel(const CUnitChannel&);
  CUnitChannel& operator=(const CUnitChannel&);
  int operator==(const CUnitChannel&) const;
  int operator!=(const CUnitChannel&) const;
public:

  // Operations on the object:

  std::string getUnitsName()  const;
  CChannel* getUnitsChannel();
  virtual void Connect();
  bool hasUnits() const;
  std::string getUnits() const;

};

#endif

