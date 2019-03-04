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

#ifndef CTRANSFORMFACTORY_H
#define CTRANSFORMFACTORY_H

#include <CBaseMediator.h>
#include <CTransformMediator.h>

#include <map>
#include <memory>
#include <utility>


namespace DAQ {
  namespace Transform {

    /*!
      * \brief The CTransformCreator class
      *
      *  Base class for all creators used by the CTransformFactories
      */
    class CTransformCreator {
    public:
      virtual std::unique_ptr<CBaseMediator> operator()() const = 0;
    };


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////


    /*! \brief Generic creator for CTransformMediator template classes
      *
      * Provided a transformer type in the template parameter, this will create
      * the appropriate type of CTransformMediator<> and return a pointer to a
      * new instance created with the default constructor
      */
    template<class T>
    class CGenericCreator : public CTransformCreator
    {
    public:
      std::unique_ptr<CBaseMediator> operator()() const {
        return std::unique_ptr<CBaseMediator>(new CTransformMediator<T>());
      }
    };

    /*!
      * \brief Extensible factory for constructing CTransformMediators
      *
      * The transform factory. Really this could work with any type of CBaseMediator, but it
      * is designed to use to and from version numbers to appropriately select the desired
      * transform.
      *
      */
    class CTransformFactory
    {
    public:
      using Container = typename std::map< std::pair<int, int>, std::unique_ptr<CTransformCreator> >;
      using Key       = typename Container::iterator;

    private:
      Container m_creators;

    public:
      CTransformFactory();

      /*!
       * \brief Sets the creator object for a specific transform type
       *
       * If a creator already exists for the pair of vsnFrom and vsnTo, this
       * swaps out the old creator for the new one. Otherwise, the creator
       * is added. The ownership of the pointer wrapped in pCreator will
       * be transferred.
       *
       * \param vsnFrom - version to transform from
       * \param vsnTo   - version to transform to
       * \param pCreator - creator object
       */
      void setCreator(int vsnFrom, int vsnTo, std::unique_ptr<CTransformCreator> pCreator);

      /*!
        * \brief Create a mediator instance
        * \param vsnFrom - version to transform from
        * \param vsnTo   - version to transform to
        * \return new instance of a mediator
        *
        * \throws std::out_of_range exception if no creator is associated with the version specifiers
        */
      std::unique_ptr<CBaseMediator> create(int vsnFrom, int vsnTo);
    };


  } // end of Transform
} // end of DAQ

#endif // CTRANSFORMFACTORY_H
