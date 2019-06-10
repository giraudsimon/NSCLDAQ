/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/




#ifndef CCOMPOSITEFILTER_H
#define CCOMPOSITEFILTER_H

#include <vector> 
#include "CFilter.h"

class CCompositeFilter : public CFilter
{
  public:
    // Basic typedefs for use with the class
    typedef std::vector<CFilter*> FilterContainer;
    typedef FilterContainer::iterator iterator;
    typedef FilterContainer::const_iterator const_iterator;

  public:
    FilterContainer m_filter; //!< The list of filters

  public:
    // Canonical methods
    CCompositeFilter();
    CCompositeFilter(const CCompositeFilter&);
    CCompositeFilter& operator=(const CCompositeFilter&);

    // Filter registration
    void registerFilter(const CFilter* filter);

    // Virtual copy constructor 
    CCompositeFilter* clone() const;

    virtual ~CCompositeFilter();

    // Handlers
    virtual CRingItem* handleRingItem(CRingItem* item);
    virtual CRingItem* handleStateChangeItem(CRingStateChangeItem* item);
    virtual CRingItem* handleScalerItem(CRingScalerItem* item);
    virtual CRingItem* handleTextItem(CRingTextItem* item);
    virtual CRingItem* handlePhysicsEventItem(CPhysicsEventItem* item);
    virtual CRingItem* handlePhysicsEventCountItem(CRingPhysicsEventCountItem* item);
    virtual CRingItem* handleFragmentItem(CRingFragmentItem* item);

    // Startup
    virtual void initialize();
    // Exit 
    virtual void finalize();

    // iterator interface
    iterator begin() { return m_filter.begin(); }
    const_iterator begin() const { return m_filter.begin(); }

    iterator end() { return m_filter.end(); }
    const_iterator end() const { return m_filter.end(); }

    void clear() { clearFilters() ;}
    size_t size() const { return m_filter.size();}

  private:
    // Free all dynamically allocated memory resize container 
    void clearFilters();

};

#endif
