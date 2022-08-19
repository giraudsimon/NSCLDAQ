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
#include "CCompoundEventSegment.h"
#include <RangeError.h>

// Local visitor classes:

/*  Initialization */

class InitializeVisitorE : public CCompoundEventSegment::CVisitor
{
public:
  virtual void operator()(CEventSegment* pSegment) {
    pSegment->initialize();
  }
};

/*
  Visitor that clears the visited elements:
*/
class ClearVisitorE : public CCompoundEventSegment::CVisitor
{
public:
  virtual void operator()(CEventSegment* pSegment) {
    pSegment->clear();
  }
};


/* 
   Visitor that disables what it visits:
*/
class DisableVisitorE : public CCompoundEventSegment::CVisitor
{
public:
  virtual void operator()(CEventSegment* pSegment) {
    pSegment->disable();
  }
};

/*
   Visitor that invokes onBegin for a segment:
*/
class BeginVisitor : public CCompoundEventSegment::CVisitor
{
public:
  virtual void operator()(CEventSegment* pSegment) {
    pSegment->onBegin();
  }
};

/*
   Visitor that invokes onEnd for a segment:
*/
class EndVisitor : public  CCompoundEventSegment::CVisitor
{
public:
  virtual void operator()(CEventSegment* pSegment) {
    pSegment->onEnd();
  }
};

/*
   Visitor that invokes onPause for a segment.
*/
class PauseVisitor : public CCompoundEventSegment::CVisitor
{
public:
  virtual void operator()(CEventSegment* pSegment) {
    pSegment->onPause();
  }
};
/*
   Visitor that invokes onResume for a segment:
*/
class ResumeVisitor : public CCompoundEventSegment::CVisitor
{
public:
  virtual void operator()(CEventSegment* pSegment) {
    pSegment->onResume();
  }

};


/*!
  Initialize the event segments by visiting them with a
  InitializeVisitor
*/
void   
CCompoundEventSegment::initialize() 
{
  InitializeVisitorE v;
  visit(v);
}
/*!
   Clear the event segments by visiting them with  a
   ClearVisitor
*/

void   
CCompoundEventSegment::clear() 
{
  ClearVisitorE v;
  visit(v);
}

/*!
   Disable the event segments by visiting themwith a 
  DisableVisitor.
*/

void   
CCompoundEventSegment::disable() 
{
  DisableVisitorE v;
  visit(v);
}

/**
 * onBegin
 *   We invoke onBegin for all component members.
 *   Traversal is recursively depth first.
 */
void
CCompoundEventSegment::onBegin()
{
  BeginVisitor v;
  visit(v);
}
/**
 * onEnd
 *   Invokes onEnd for all component members.
 */
void
CCompoundEventSegment::onEnd()
{
  EndVisitor v;
  visit(v);
}
/**
 * onPause
 *    Invokes onPause for all component members.
 */
void
CCompoundEventSegment::onPause()
{
  PauseVisitor v;
  visit(v);
}
/**
 * onResume
 *    Invokes onResume for all component members.
 */
void
CCompoundEventSegment::onResume()
{
  ResumeVisitor v;
  visit(v);
}

/*!
  Read can't be done with a visitor in a copy free manner.
  We'll iterate here calling read which in turn will recurse as needed,
  advancing the pointer and summing the sizes..
  if a size would overflow, we'll throw the CRangeError which
  hopefully will prevent much further damage.

  \param pBuffer - Pointer to storage sufficient to hold the event.
  \param maxwords - Number of uint16_t pBuffer can hold.
  \return size_t
  \retval Number of uint16_t actually read.
  \throw CRangeError if an attempt was made to read in excess of maxwords.
*/
size_t 
CCompoundEventSegment::read(void* pBuffer, size_t maxwords) 
{
  uint16_t*  pD        = reinterpret_cast<uint16_t*>(pBuffer);
  size_t     remaining = maxwords;
  size_t     nRead     = 0;

  EventSegmentIterator p = begin();
  while (p != end()) {
    CEventSegment* pSeg = dynamic_cast<CEventSegment*>(*p);
    pSeg->keep();
    size_t segSize      = pSeg->read(pD, remaining);
    if (segSize > remaining) {
      throw CRangeError(0, maxwords, nRead + segSize, 
			"Reading an event");
    }
    nRead     += segSize;
    remaining -= segSize;
    pD        += segSize;

    CEventSegment::AcceptState acceptance = pSeg->getAcceptState();
    
    // If the segment wants to quit honor that and set that as our state.

    if (acceptance == CEventSegment::RejectImmediately) {
      rejectImmediately();
      break;
    }
    
    // If the segwments wants to reject but keep processing, just propogate
    // that state.

    if (acceptance == CEventSegment::Reject) {
      reject();
    }

    p++;
  }
  return nRead;
    
}


// Type-safe adaptor to CComposite:

/*!
  Type safe way to add an event segment.  Adding through here prevents
  e.g. adding a scaler bank.

  \param pSegment - the segment to add.

*/
void 
CCompoundEventSegment::AddEventSegment(CEventSegment*    pSegment) 
{
  addItem(pSegment);
}

/*!
  Type-safe way to remove an event segment.  

  \param pSegment - The segment to remove

  \note It is a legal no-op to attempt to remove a segment
        that is notin the container.
  \note It is the caller's responsibility to free any dynamically allocated
        event segment if necessary.
*/
void 
CCompoundEventSegment::DeleteEventSegment(CEventSegment* pSegment) 
{
  deleteItem(pSegment);
}

/*!
  Return a begin of iteration iterator for the collection
*/

CCompoundEventSegment::EventSegmentIterator 
CCompoundEventSegment::begin() 
{
  return CComposite::begin();
}
/*!
  Return an end of iteration iterator for the collection:
*/
CCompoundEventSegment::EventSegmentIterator 
CCompoundEventSegment::end() 
{
  return CComposite::end();
}
/*!
  Indicate that we are, in fact, a composite:
*/
bool 
CCompoundEventSegment::isComposite() const 
{
  return true;
}


  
/*!
  Visit all items in the container.  If recursion is required/desired either
  the visitor must do it, or the member functions it calls must.  For example,
  The initialization visitor will invoke the visited node's initialize which,
  if the node is compound will take care of recursing by doing the visit on its
  children.
*/
void CCompoundEventSegment::visit(CVisitor& visitor)
{
  EventSegmentIterator p = begin();
  while(p != end()) {
    CEventSegment* pSegment = dynamic_cast<CEventSegment*>(*p);
    visitor(pSegment);
    p++;
  }
}
