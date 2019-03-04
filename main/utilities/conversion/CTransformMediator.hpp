/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <iostream>
#include <CDataSource.h>
#include <CDataSink.h>
#include <CCompositePredicate.h>
#include <V10/CRingItem.h>
#include <V11/CRingItem.h>
#include <RingIOV10.h>
#include <RingIOV11.h>

#include <iostream>
#include <stdexcept>


namespace DAQ {
  namespace Transform {

/**! Constructor

  Constructs the mediator object. This object owns its referenced members.

  \param source a pointer to a CDataSource
  \param filter a pointer to a CFilter
  \param sink a pointer to a CDataSink

*/
template<class Transform>
CTransformMediator<Transform>::CTransformMediator(std::unique_ptr<CDataSource> source,
                                       std::unique_ptr<CDataSink> sink,
                                       Transform transform)
: CBaseMediator(move(source), move(sink)),
  m_transform(transform),
  m_pPredicate(new CCompositePredicate)
{}

/**! Destructor
  Delete the owned objects. It is possible that these were never initialized,
  so it is important to verify that they point to something other than NULL.

  Following a delete the pointers are reset to 0. This protects against double
  frees when a user decides to call an objects destructor explicitly and then
  it goes out of scope.
*/
template<class Transform>
CTransformMediator<Transform>::~CTransformMediator()
{
}

/**! The main loop
  This is the workhorse of the application. Items are retrieved 
  from the source, passed to the filter, and then the item returned 
  by the filter is sent to the
  sink. During each iteration through this process, the item retrieved
  from the source and the filter are properly managed.
*/
template<class Transform>
void CTransformMediator<Transform>::mainLoop()
{
  if (! getDataSource()) {
    std::string errmsg = "CTransformMediator<>::mainLoop() Data source is null";
    throw std::runtime_error(errmsg);
  }
  if (! getDataSink()) {
    std::string errmsg = "CTransformMediator<>::mainLoop() Data sink is null";
    throw std::runtime_error(errmsg);
  }

  // Dereference our pointers before entering
  // the main loop

  while (processOne()) {}

}
template<class Transform>
void CTransformMediator<Transform>::initialize()
{
}
template<class Transform>
void CTransformMediator<Transform>::finalize()
{
}
template<class Transform>
bool CTransformMediator<Transform>::processOne()
{
  CDataSource& source    = *getDataSource();
  CDataSink&   sink      = *getDataSink();

  using T1 = typename Transform::InitialType;
  using T2 = typename Transform::FinalType;

  T1 item1(1);
  source >> item1;

  if (source.eof()) {
    return false;
  }

  updatePredicate();

  try {

      T2 item2 = m_transform(item1);

      if (item2.type() != 0) {
        sink << item2;
      }
  } catch (std::exception& exc) {
    std::cout << exc.what() << std::endl;
  } catch (...) {
    std::cout << "Caught an error" << std::endl;
  }

  return true;
}

template<class Transform>
void CTransformMediator<Transform>::updatePredicate()
{}

  } // end of Transform
} // end of DAQ

