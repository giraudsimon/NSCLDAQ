//  Trace.hpp
//
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#ifndef TRACE_H
#define TRACE_H

#include <stdint.h>
#include <vector>
#include <TObject.h>
#include "TrIterator.hpp"

namespace TrAnal 
{

template<class DataType>
class Basic_Trace
{
    public:
        virtual ~Basic_Trace() {};

        virtual TrIterator<DataType> begin() const = 0;
        virtual TrIterator<DataType> end() const = 0;

        virtual DataType operator[](unsigned int index) const = 0;
        virtual size_t GetLength() const = 0;

    // ROOT dictionary generation
    /// \cond
    ClassDef(Basic_Trace,1);
    /// \endcond
};


// A template class 
// The intention is to support arbitrary stl vector but any 
// class can be used to instantiate this so long as it supports
template<class T>
class TraceT : public Basic_Trace<T>
{
    private:
    std::vector<T>    fData;

    public:
    // Canonical constructors
    TraceT() : Basic_Trace<T>(), fData() {}
    TraceT(const std::vector<T>& obj) : Basic_Trace<T>(), fData(obj) {}

    // destructor
    virtual ~TraceT() {}


    TrIterator<T> begin() const { return TrIterator<T>(fData.data());}
    TrIterator<T> end() const { return TrIterator<T>(fData.data()+fData.size());}
    //
    virtual T operator[] (unsigned int index) const;
    virtual size_t GetLength() const { return fData.size(); } 

    // ROOT dictionary generation
    /// \cond
    ClassDef(TraceT,1);
    /// \endcond
};

template<class T>
T TraceT<T>::operator[](unsigned int index) const 
{
    if (index>=fData.size()) 
        throw 1;
    else   
        return fData.at(index);

}
//
//// A template class with calibration to support time definitions
//// The intention is to support arbitrary stl vector but any 
//// class can be used to instantiate this so long as it supports
//template<class T>
//class CalibTraceT : public Trace 
//{
//    private:
//    std::vector<T>    fData;  ///< the data points 
//    double            fSlope; ///< time difference between points
//    double            fOffset; ///< time of first point 
//
//    public:
//    /// Default constructor
//    /**
//    *   Constructs null trace with a unit time calibration (slope=1.0, offset=0.0)
//    *   There is no data in the trace produced by this constructor
//    */
//    TraceT() : Trace(), fData(), fSlope(1.0), fOffset(0.0) {}
//
//    /// Detailed constructor
//    /**
//    *  Constructs a calibration trace by copying the argument data
//    *  The slope and offset are 1.0 and 0.0 by default but can be set 
//    *  differently
//    */
//    TraceT(const std::vector<T>& obj, double slope=1.0, double offset=0.0) 
//        : Trace(), fData(obj), fSlope(slope), fOffset(offset) {}
//
//    virtual ~TraceT() {}
//
//    /// Get the iterator for this
//    /** 
//    *   This passes ownership of the iterator to the caller. It is possible
//    *   to have multiple iterators for the same trace.
//    *
//    *   @return a calibrated iterator object   
//    */
//    CalibratedRangeIterator* GetIterator() const { return new CalibratedRangeIterator(*this, fSlope, fOffset); }
//    virtual double operator[] (unsigned int index) const;
//    virtual unsigned int GetLength() const { return fData.size(); } 
//
//    /// Get the time between successive points
//    double GetSlope() const { return fSlope;}
//
//    /// Time of first point
//    double GetOffset() const { return fOffset;}
//
//    
//};
//
//template<class T>
//double CalibTraceT<T>::operator[](unsigned int index) const 
//{
//    if (index>=fData.size()) 
//        throw 1;
//    else   
//        return fData.at(index);
//
//}

} // end namespace
#endif
