/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2018.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Giordano Cerizza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#ifndef __SCALERREADERMAIN_H
#define __SCALERREADERMAIN_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#include <fstream>

// forward class definitions:
class CRingScalerItem;
class CRingStateChangeItem;
class CRingTextItem;
class CPhysicsEventItem;
class CRingPhysicsEventCountItem;
class CDataFormatItem;
class CGlomParameters;
class CRingItem;

class scalerReaderMain {
  
public:
  scalerReaderMain();
  virtual ~scalerReaderMain();
  
  int operator()(int argc, char** argv);

private:

  std::ofstream outscalers;
  int scalerBuffer;
  int scalernumbers[32];
  
  void processRingItem(CRingItem& item);
  void processScalerItem(const CRingScalerItem& item);
  void processStateChangeItem(CRingStateChangeItem& item);
  void processTextItem(CRingTextItem& item);
  void processEvent(CPhysicsEventItem& item);
  void processEventCount(CRingPhysicsEventCountItem& item);
  void processFormat(CDataFormatItem& item);
  void processGlomParams(CGlomParameters& item);
  void processUnknownItemType(CRingItem& item);
  
};

#endif
