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
#include "CScalerBank.h"
#include <fragment.h>

using namespace std;

// Utility classes


// Visitor to initialize all children.

class InitializeVisitor : public CScalerBank::CVisitor
{
public:
  virtual void operator()(CScaler* pItem) {
    pItem->initialize();
  }
};

// Visitor to clear children:

class ClearVisitor : public CScalerBank::CVisitor
{
public:
  virtual void operator()(CScaler* pItem) {
    pItem->clear();
  }
};

// Visitor to disable children:

class DisableVisitor : public CScalerBank::CVisitor
{
public:
  virtual void operator()(CScaler* pItem) {
    pItem->disable();
  }
};

// Visitor to read children:

class ReadVisitor : public CScalerBank::CVisitor
{
private:
  vector<uint32_t> m_values;
  
public:
  virtual void operator()(CScaler* pItem) {
    vector<uint32_t> values = pItem->read();
    m_values.insert(m_values.end(),
		    values.begin(), values.end());
  }
  vector<uint32_t> getScalers() const
  {
    return m_values;
  }
};

// Visitor to get timestamp:

class TimestampVisitor : public CScalerBank::CVisitor
{
private:
  uint64_t m_timestamp;
public:
  TimestampVisitor() : m_timestamp(NULL_TIMESTAMP) {}
  virtual void operator() (CScaler* pItem) {
    uint64_t t = pItem->timestamp();
    if (t != NULL_TIMESTAMP) m_timestamp = t;
  }
  uint64_t timestamp() const {
    return m_timestamp;
  }
};

// Visitor to get source id:

class SourceIdVisitor : public CScalerBank::CVisitor
{
private:
  int m_id;
public:
  SourceIdVisitor() : m_id(-1) {}
  virtual void operator() (CScaler* pItem) {
    int id = pItem->sourceId();
    if (id != -1) m_id = id;
  }
  int getId() const {return m_id; }
};

  // Members inherited from CScaler:

/*!
  Deep initiailization of the modules:
*/
void 
CScalerBank::initialize()
{
  InitializeVisitor i;
  visit(i);
}

/*!
  Deep initialization of the modules
*/
void 
CScalerBank::clear()
{
  ClearVisitor v;
  visit(v);
}
/*!
  Deep disable of the hierarchy:
*/
void 
CScalerBank::disable()
{
  DisableVisitor v;
  visit(v);
}
/*!
  Read the data from the scaler modules.
  Return it as a vector of int32's.
*/
std::vector<uint32_t> 
CScalerBank::read()
{
  ReadVisitor v;
  visit(v);
  return v.getScalers();

}
 /**
  * timestamp
  *   Returnt a timestamp for the scaler bank.
  *   Each scaler is visited.  The timestamp returned by the last scaler
  *   not returning NULL_TIMESTAMP wins.   If no scaler returns a non
  *   NULL_TIMESTAMP stamp, NULL_TIMESTAMP is used.
  *   See TimestampVisitor
  */
uint64_t
CScalerBank::timestamp()
{
  TimestampVisitor tstamp;
  visit(tstamp);
  return tstamp.timestamp();
}

/**
 * sourceId
 *    Return the source id associated with the scaler event.
 *    This is done by visiting each scaler and asking it for its source id.
 *    The last one that is not -1 is returned.  If none are -1
 *    then -1 is returned.  The framework treats a -1 return as a request to
 *    use the source id specified on the command line (--sourceid).
 */
int
CScalerBank::sourceId()
{
  SourceIdVisitor v;
  visit(v);
  return v.getId();
}

// Type safe adapter to CComposite:


/*!
  Add a scaler module to the collection:
  \param pScaler - Pointer to the module to add.

  \note The clients are resposible for any storage management
        if pScaler points to a dynamically allocated object.

*/
void 
CScalerBank::AddScalerModule(CScaler* pScaler)
{
  CComposite::addItem(pScaler);
}
/*!
   Delete a scaler from the collection.
   \param pScaler - Points to the item delete.

   \note It is up to upper layers of software to worry
        about handling storage management in the case when
	pScaler was dynamically allocated.
*/
void 
CScalerBank::DeleteScaler(CScaler* pScaler)
{
  CComposite::deleteItem(pScaler);
}

/*!
   Returns a begin iteration to the container.
*/
CScalerBank::ScalerIterator 
CScalerBank::begin()
{
  CScalerBank::ScalerIterator p = CComposite::begin();
  return p;
}
/*!
  Returns an end iteration to the container of scalers:
*/
CScalerBank::ScalerIterator 
CScalerBank::end()
{
  CScalerBank::ScalerIterator p = CComposite::end();
  return p;
}


/*!
   This is a composite:
*/
bool
CScalerBank::isComposite() const
{
  return true;
}


/*!
  Vists every element in the collection.  Does not recurse.
  It's up tothe caller to arrange recursion if needed.
  Note that usually the member functions of the collections
  handle recursion.

  \param visitor - A reference to a function object that is called once
                   for each element of the collection. The function object
		   is passed a pointer to the object being visited as a
		   sole parameter.  The pointer will be of type CScaler*.

Here's a simple example of counting elements in the collection with recursion;
Using this is equivalent to fullCount():


\verbatim
class counter : public CScalerBank::Visitor {
  int  m_count;
public:
  CScalerBank() : m_count(0) {}
  int getCount() {return m_count; }
  virtual void operator()(CScaler* pObject) {
     m_count++;
     if (pObject->isComposite()) {
       CScalerBank* pBank = dynamic_cast<CScalerBank*>(pObject);
       counter nextDown;
       pBank->visit(nextDown);
       m_count += nextDown.getCount();
     }
  }
};

...

// Count the number of items in CScalerBank bank:

counter totalCount;
bank.visit(totalCount);
cerr << "There are " << totalCount.getCount() << "elements\n";

\endverbatim
*/
void 
CScalerBank::visit(CScalerBank::CVisitor& visitor)
{
  ScalerIterator p = begin();
  while(p != end()) {
    CScaler* pScaler = dynamic_cast<CScaler*>(*p);
    visitor(pScaler);

    p++;
  }
}
