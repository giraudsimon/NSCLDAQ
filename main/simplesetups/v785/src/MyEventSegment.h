
/*
  This is the header file to define the MyEventSegment class, which
  is derived from CEventSegment. This class can be used to read
  out any number of CAEN modules covered by the CAENcard class.
  Those cards include the V785, V775, and V792.
  6Chapter 2. Setting up the software
  Tim Hoagland
  11/3/04
  s04.thoagland@wittenberg.edu
*/

#ifndef MYEVENTSEGMENT_H                          
#define MYEVENTSEGMENT_H

#include <CEventSegment.h>
#include <CDocumentedPacket.h>
#include <CAENcard.h>

// Declares a class derived from CEventSegment
class MyEventSegment : public CEventSegment      
{
  private:
    CDocumentedPacket m_myPacket;               
    CAENcard          m_module;                

  public:
    MyEventSegment(short slot, unsigned short Id);
    virtual void initialize();                    
    virtual void clear();                         
    virtual size_t read(void* pBuffer, size_t maxwords);

  public:
    uint64_t computeTimestamp(uint16_t* pBuf);
};
#endif 
