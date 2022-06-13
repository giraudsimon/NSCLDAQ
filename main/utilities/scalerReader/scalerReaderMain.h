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
#ifndef SCALERREADERMAIN_H
#define SCALERREADERMAIN_H

#include <string>
#include <vector>
#include <map>
#include <stdint.h>
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
private:
  void dumpScalers();
  void header(const char* title, std::string nRun);
};

#endif
