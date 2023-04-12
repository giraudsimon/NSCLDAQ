#ifndef __BUFDUMPMAIN_H
#define __BUFDUMPMAIN_H

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

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CPP_OSTREAM
#include <ostream>
#ifndef __CPP_STD_OSTREAM
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif


// Forward definitiosn:

class URL;
class CRingItem;
class CRingStateChangeItem;
class CRingTextItem;
class CRingScalerItem;
class CRingPhysicsEventCountItem;
class CPhysicsEventItem;

class CRingBuffer;

class Converter;

class RingItemFactoryBase;


/*!
  This class is the actual dumper program.  It defines a function object type
  that can be created and invoked from main() to do the job of dumping
  items from some item source on stdout.

  See dumperargs.ggo for information about the switches etc. recognized by this
  program/object.

*/
class BufdumpMain
{
  // private data:
  
private:
  bool                  m_ringSource;// True if the data source is a ring buffer.
  URL*                  m_pDataSource;  // URI that defines the data source.
  size_t                m_skipCount;   // Number  of items to skip before dumping.
  size_t                m_itemCount;   // Number of items to dump before exiting (0 means infinite).
  Converter         *m_prootconverter;  // converter to ROOT file
  std::vector<uint16_t> m_sampleTypes; // Items that should be sampled only (ring buffers).
  std::vector<uint16_t> m_excludeTypes; // Items that should not be dumped at all.
  RingItemFactoryBase*  m_pRingItemFactory;  // Makes us format independent.

  // Canonicals... no need for copy construction etc.
  //
public:
  BufdumpMain();
  virtual ~BufdumpMain();
protected:
  BufdumpMain(const BufdumpMain& rhs);
  BufdumpMain& operator=(const BufdumpMain& rhs);
  int operator==(const BufdumpMain& rhs) const;
  int operator!=(const BufdumpMain& rhs) const;
public:

  // Entry point:

  int operator()(int argc, char** argv);

  //Utilities:

private:
  CRingItem* getItem(CRingBuffer& ring);
  void processItem(const CRingItem& item);
  void dumpStateChangeItem(std::ostream& out, const CRingStateChangeItem& item);
  void dumpStringListItem(std::ostream&  out, const CRingTextItem& item);
  void dumpScalerItem(std::ostream& out, const CRingScalerItem& item);
  void dumpPhysicsItem(std::ostream& out, const CPhysicsEventItem& item);
  void dumpEventCountItem(std::ostream& out, const CRingPhysicsEventCountItem& item);
  void dumpUnknownItem(std::ostream& out, const CRingItem& item);

  std::string defaultSource() const; 
  std::string timeString(time_t theTime) const;
  
};

#endif
