//  Definition.h
//  
//  Author : Jeromy Tompkins
//  Date   : 8/14/2013

#ifndef DEFINITIONS
#define DEFINITIONS

#include <stdint.h>
#include "Trace.hpp"
#include "SumIterator.hpp"
#include "TrapFilter.hpp"

namespace TrAnal 
{

// Typedef some useful type names that are most likely to be used

/// \brief Trace containing uint16_t type data
typedef TraceT<uint16_t> TraceS;

/// \brief TrIterator for uint16_t type data
typedef TrIterator<uint16_t> TrIterS;

/// \brief TrRange for uint16_t type data 
typedef TrRange<uint16_t> TrRangeS;

/// \brief SumIterator for uint16_t type data 
typedef SumIterator<uint16_t> SumIterS;

/// \brief TrapFilter for uint16_t type data 
typedef TrapFilter<uint16_t> TrFilterS;
}

#endif

