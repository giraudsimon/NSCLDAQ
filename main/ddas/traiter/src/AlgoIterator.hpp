//  AlgoIterator.hpp
//
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#ifndef ALGOITERATOR_H
#define ALGOITERATOR_H

#include "Trace.hpp"
#include "TObject.h"

namespace TrAnal
{

    /// Abstract AlgoIterator base class
    template<class T> 
    class AlgoIterator 
    {
        public:
        /// \brief Default constructor
        AlgoIterator() {}
        /// \brief Copy constructor
        AlgoIterator(const AlgoIterator& ) {}
        /// \brief Assignment operator
        AlgoIterator& operator=(const AlgoIterator& ) { return *this;}
        virtual ~AlgoIterator() {}

        // Support for basic comparison with TrIterator
        virtual bool operator<(const TrIterator<T>& ) const { return false; }
 
        // Support for range specification
        /** \brief Min extent
        *   To implement the minimum iterator extent for the range
        *   This default implementation returns an iterator to address 0x0 
        *   @return null iterator
        */
        virtual TrIterator<T> min_extent() const { return TrIterator<T>(0);}

        /** \brief Max extent
        *   To implement the maximum iterator extent for the range
        *   This default implementation returns an iterator to address 0x0 
        *   @return null iterator
        */
        virtual TrIterator<T> max_extent() const { return TrIterator<T>(0);}
    
        // Support for incrementation
        /**  \brief Prefixed incrementation 
        *  Supports ++x style incrementing. Default implementation does nothing.
        *   @return const reference to this
        */
        virtual AlgoIterator& operator++() { return *this;}

        // Support for dereferencing to a double
        /** \brief Dereference operator
        *
        * Contrary to TrIterators, this returns a double value irrespective of
        * the template parameter. This return 0 always.
        * @return null value (i.e. zero)
        */
        virtual double operator*() const { return 0;}
        
        // ROOT dict generation
        /// \cond
        ClassDef(AlgoIterator,0);
        /// \endcond
    }; // end AlgoIterator class

} // end of namespace

#endif
